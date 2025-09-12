/*---------------------------------------------------------------
 *	タスク遅延の例題の動作確認プログラム( μ T-Kernel 3.0用)
 *
 *	Copyright (C) 2022 by T3 WG of TRON Forum
 *---------------------------------------------------------------*/
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

#define LONG_MIN 0x80000000	/* 整数の最小値を特殊な意味で使用 */
#define NO_PARAM LONG_MIN	/* ログのパラメータが無い場合の指定 */

/* ログを記録するための配列(21) */
#define	MAX_LOG	50
UB	*log_msg[MAX_LOG];
W	log_param[MAX_LOG];
UW	log_time[MAX_LOG];
W	log_index = 0;

/* ログを記録するための関数(22) */
void log(UB *msg, W param)
{
	SYSTIM	tim;
	tk_get_tim(&tim);		/* 現在時刻を取得 */
	log_msg[log_index] = msg;	/* ログメッセージを保存 */
	log_param[log_index] = param;	/* ログメッセージのパラメータを保存 */
	log_time[log_index] = tim.lo;	/* ミリ秒単位の時刻の下位32ビットを保存 */
	if(++log_index >= MAX_LOG){	/* ログ記録用の配列のインデクスを更新 */
		tm_printf((UB*) "*** log overflow !\n");
		log_index--;
	}
}

/* 最後にまとめてログを表示する関数(23) */
void print_log(void)
{
	UB	msg_param[16];
	W	cnt, dtime;

	for(cnt = 0 ; cnt < log_index ; cnt++){

		/* usermain実行開始時からの経過時間(相対時刻)を計算(24) */
		dtime = log_time[cnt] - log_time[0];

		tm_sprintf(msg_param, "");	/* ログにパラメータが無い場合 */
		if(log_param[cnt] != NO_PARAM){	/* パラメータ(整数)がある場合 */
			tm_sprintf(msg_param, "%d", log_param[cnt]);
		}
		tm_printf((UB*) "%4d:%6dms: %s%s\n",
			(cnt + 1), dtime, log_msg[cnt], msg_param);
	}
}

/* オブジェクトID番号 */
ID tidA;	/* タスクAのID */
ID tidB;	/* タスクBのID */
ID tidC;	/* タスクCのID */

/* タスクA */
void taskA( INT stacd, void *exinf )
{
	log("Task A: start", NO_PARAM);
	tk_dly_tsk( 10 );		/* 10msディレイ(8) */

	log("Task A: exit and delete", NO_PARAM);
	tk_exd_tsk();			/* 自タスクを終了・削除(12) */
}

/* タスクB */
void taskB( INT stacd, void *exinf )
{
	log("Task B: start", NO_PARAM);
	tk_dly_tsk( 100 );		/* 100msディレイ(10) */

	log("Task B: exit and delete", NO_PARAM);
	tk_exd_tsk();			/* 自タスクを終了・削除(16) */
}

/* タスクC */
void taskC( INT stacd, void *exinf )
{
	log("Task C: start", NO_PARAM);

	tk_sta_tsk( tidB, 0 );		/* タスクBを起動(6) */
	tk_sta_tsk( tidA, 0 );		/* タスクAを起動(7) */
	tk_dly_tsk( 50 );		/* 50msディレイ(9) */

	log("Task C: exit and delete", NO_PARAM);
	tk_exd_tsk();			/* 自タスクを終了・削除(14) */
}

/* usermainによる初期化処理 */
const T_CTSK ctskA = {0, (TA_HLNG | TA_RNG3), &taskA, 10, 1024, 0};
const T_CTSK ctskB = {0, (TA_HLNG | TA_RNG3), &taskB, 20, 1024, 0};
const T_CTSK ctskC = {0, (TA_HLNG | TA_RNG3), &taskC, 20, 1024, 0};

EXPORT INT usermain( void )
{
	/* usermain実行開始を示す最初のログ(1) */
	log("Start User-main program", NO_PARAM);

	tidA = tk_cre_tsk( &ctskA );	/* タスクA(優先度10)を生成(2) */
	tidB = tk_cre_tsk( &ctskB );	/* タスクB(優先度20)を生成(3) */
	tidC = tk_cre_tsk( &ctskC );	/* タスクC(優先度20)を生成(4) */
	tk_sta_tsk( tidC, 0 );		/* タスクCを起動(5) */

	tk_dly_tsk( 1000 );		/* 終了後に1秒の時間待ち */
	log("End User-main program", NO_PARAM);	/* 実行終了のログ */
	print_log();			/* ログを表示 */

	tk_slp_tsk( TMO_FEVR );
	return 0;
}
