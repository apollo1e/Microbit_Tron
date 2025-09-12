// /*
//  *----------------------------------------------------------------------
//  *    micro T-Kernel 3.00.05
//  *
//  *    Copyright (C) 2006-2021 by Ken Sakamura.
//  *    This software is distributed under the T-License 2.2.
//  *----------------------------------------------------------------------
//  *
//  *    Released by TRON Forum(http://www.tron.org) at 2021/11.
//  *
//  *----------------------------------------------------------------------
//  */

// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>

// /* ---------------------------------------------------------
//  * Sample User Program
//  * ---------------------------------------------------------
//  * 
//  * Entry routine for the user application.
//  * At this point, Initialize and start the user application.
//  *
//  * Entry routine is called from the initial task for Kernel,
//  * so system call for stopping the task should not be issued 
//  * from the contexts of entry routine.
//  * We recommend that:
//  * (1)'usermain()' only generates the user initial task.
//  * (2)initialize and start the user application by the user
//  * initial task.
//  */

// #if USE_TMONITOR
// #define TM_PUTSTRING(a)	tm_putstring(a)

// void print_err( UB* str, ER err)
// {
// 	tm_printf(str, err);
// }

// #else
// #define TM_PUTSTRING(a)

// void print_err( UB* str, INT par) {}

// #endif /* USE_TMONITOR */

// /* ----------------------------------------------------------
//  *
//  * User Task-1 Definition
//  *
//  */
// void tsk1(INT stacd, void *exinf)
// {
// 	TM_PUTSTRING((UB*)"Start Task-1\n");

// 	tk_exd_tsk();	/* Exit task */
// }

// /* ---------------------------------------------------------
//  *
//  * User Task-2 Definition
//  *
//  */
// void tsk2(INT stacd, void *exinf)
// {
// 	TM_PUTSTRING((UB*)"Start Task-2\n");

// 	tk_exd_tsk();	/* Exit Task */
// }

// const T_CTSK	ctsk1	= {0, (TA_HLNG | TA_RNG3), &tsk1, 10, 1024, 0};
// const T_CTSK	ctsk2	= {0, (TA_HLNG | TA_RNG3), &tsk2, 11, 1024, 0};

// /* ----------------------------------------------------------
//  *
//  * User-Main Definition (Run on initial task)
//  *
//  */

// EXPORT INT usermain( void )
// {
// 	T_RVER	rver;
// 	ID	id1, id2;

// 	TM_PUTSTRING((UB*)"Start User-main program.\n");

// 	tk_ref_ver(&rver);		/* Get the OS Version. */

// #if USE_TMONITOR
// 	tm_printf((UB*)"Make Code: %04x  Product ID: %04x\n", rver.maker, rver.prid);
// 	tm_printf((UB*)"Product Ver. %04x\nProduct Num. %04x %04x %04x %04x\n", 
// 			rver.prver, rver.prno[0],rver.prno[1],rver.prno[2],rver.prno[3]);
// #endif

// 	id1 = tk_cre_tsk(&ctsk1);
// 	tk_sta_tsk(id1, 0);

// 	id2 = tk_cre_tsk(&ctsk2);
// 	tk_sta_tsk(id2, 0);

// 	tk_slp_tsk(TMO_FEVR);

// 	return 0;
// }


/*----------------------------------------------------------------------
* Task delay example verification program (for μT-Kernel 3.0)
*
* Copyright (C) 2022 by T3 WG of TRON Forum
*----------------------------------------------------------------------*/

// #include
// #include

// #define LONG_MIN 0x80000000 /* Use the minimum integer value in a special way */
// #define NO_PARAM LONG_MIN /* Specify when there is no log parameter */

// /* Array for recording logs(21) */
// #define MAX_LOG 50
// UB *log_msg[MAX_LOG];
// W log_param[MAX_LOG];
// UW log_time[MAX_LOG];
// W log_index = 0;

// /* Function for recording logs (22) */
// void log(UB *msg, W param)
// {
// SYSTIM tim;
// tk_get_tim(&tim); /* Get current time */
// log_msg[log_index] = msg; /* Save the log message */
// log_param[log_index] = param; /* Save the parameters of the log message */
// log_time[log_index] = tim.lo; /* Stores the lowest 32 bits of the time in milliseconds */
// if(++log_index >= MAX_LOG){ /* Update the index of the log array */
// tm_printf((UB*) "*** log overflow !\n");
// log_index--;
// }
// }

// /* Function to display logs at the end (23) */
// void print_log(void)
// {
// UB msg_param[16];
// W cnt, dtime;

// for(cnt = 0 ; cnt < log_index ; cnt++){

// /* Calculate the elapsed time (relative time) from the start of usermain execution (24) */
// dtime = log_time[cnt] - log_time[0];

// tm_sprintf(msg_param, ""); /* If there are no parameters in the log */
// if(log_param[cnt] != NO_PARAM){ /* If parameter (integer) exists */
//   tm_sprintf(msg_param, "%d", log_param[cnt]);
// }
// tm_printf((UB*) "%4d:%6dms: %s%s\n",
//   (cnt + 1), dtime, log_msg[cnt], msg_param);
// }
// }

// /* Object ID number */
// ID tidA; /* ID of task A */
// ID tidB; /* ID of task B */
// ID tidC; /* ID of task C */

// /* Task A */
// void taskA( INT stacd, void *exinf )
// {
// log("Task A: start", NO_PARAM);
// tk_dly_tsk( 10 ); /* 10ms delay(8) */

// log("Task A: exit and delete", NO_PARAM);
// tk_exd_tsk(); /* Terminate and delete own task (12) */
// }

// /* Task B */
// void taskB( INT stacd, void *exinf )
// {
// log("Task B: start", NO_PARAM);
// tk_dly_tsk( 100 ); /* 100ms delay(10) */

// log("Task B: exit and delete", NO_PARAM);
// tk_exd_tsk(); /* Terminate and delete own task (16) */
// }

// /* Task C */
// void taskC( INT stacd, void *exinf )
// {
// log("Task C: start", NO_PARAM);

// tk_sta_tsk( tidB, 0 ); /* Start task B (6) */
// tk_sta_tsk( tidA, 0 ); /* Start task A (7) */
// tk_dly_tsk( 50 ); /* 50 ms delay(9) */

// log("Task C: exit and delete", NO_PARAM);
// tk_exd_tsk(); /* Terminate and delete own task (14) */
// }

// /* Initialization process by usermain */
// const T_CTSK ctskA = {0, (TA_HLNG | TA_RNG3), &taskA, 10, 1024, 0};
// const T_CTSK ctskB = {0, (TA_HLNG | TA_RNG3), &taskB, 20, 1024, 0};
// const T_CTSK ctskC = {0, (TA_HLNG | TA_RNG3), &taskC, 20, 1024, 0};

// EXPORT INT usermain( void )
// {
// /* The first log indicating the start of usermain execution (1) */
// log("Start User-main program", NO_PARAM);

// tidA = tk_cre_tsk( &ctskA ); /* Create task A (priority 10) (2) */
// tidB = tk_cre_tsk( &ctskB ); /* Create task B (priority 20) (3) */
// tidC = tk_cre_tsk( &ctskC ); /* Create task C (priority 20) (4) */
// tk_sta_tsk( tidC, 0 ); /* Start task C (5) */

// tk_dly_tsk( 1000 ); /* Wait 1 second after termination */
// log("End User-main program", NO_PARAM); /* Execution end log */
// print_log(); /* Display the log */

// tk_slp_tsk( TMO_FEVR );
// return 0;
// }#include <tk/tkernel.h>
/*---------------------------------------------------------------
 *	タスク遅延の例題の動作確認プログラム( μ T-Kernel 3.0用)
 *
 *	Copyright (C) 2022 by T3 WG of TRON Forum
 *---------------------------------------------------------------*/
// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>

// #define MSGLEN 100		/* メッセージ長 */
// #define MBFBUFSZ 1000		/* メッセージバッファのサイズ */

// #define LONG_MIN 0x80000000	/* 整数の最小値を特殊な意味で使用 */
// #define NO_PARAM LONG_MIN	/* ログのパラメータが無い場合の指定 */

// /* ログを記録するための配列(21) */
// #define	MAX_LOG	50
// UB	*log_msg[MAX_LOG];
// W	log_param[MAX_LOG];
// UW	log_time[MAX_LOG];
// W	log_index = 0;

// /* ログを記録するための関数(22) */
// void log(UB *msg, W param)
// {
// 	SYSTIM	tim;
// 	tk_get_tim(&tim);		/* 現在時刻を取得 */
// 	log_msg[log_index] = msg;	/* ログメッセージを保存 */
// 	log_param[log_index] = param;	/* ログメッセージのパラメータを保存 */
// 	log_time[log_index] = tim.lo;	/* ミリ秒単位の時刻の下位32ビットを保存 */
// 	if(++log_index >= MAX_LOG){	/* ログ記録用の配列のインデクスを更新 */
// 		tm_printf((UB*) "*** log overflow !\n");
// 		log_index--;
// 	}
// }

// /* 最後にまとめてログを表示する関数(23) */
// void print_log(void)
// {
// 	UB	msg_param[16];
// 	W	cnt, dtime;

// 	for(cnt = 0 ; cnt < log_index ; cnt++){

// 		/* usermain実行開始時からの経過時間(相対時刻)を計算(24) */
// 		dtime = log_time[cnt] - log_time[0];

// 		tm_sprintf(msg_param, "");	/* ログにパラメータが無い場合 */
// 		if(log_param[cnt] != NO_PARAM){	/* パラメータ(整数)がある場合 */
// 			tm_sprintf(msg_param, "%d", log_param[cnt]);
// 		}
// 		tm_printf((UB*) "%4d:%6dms: %s%s\n",
// 			(cnt + 1), dtime, log_msg[cnt], msg_param);
// 	}
// }

// /* オブジェクトID番号 */
// ID tidA;	/* タスクAのID */
// ID tidB;	/* タスクBのID */
// ID mbfidX;	/*　メッセージバッファXのID */

// /* タスクA */
// void taskA(INT stacd, void *exinf)
// {
// 	INT i, k;
// 	UB msg[MSGLEN];

// 	log("Task A: start", NO_PARAM);

// 	for (i = 0; i < 5; i++) {

// 		/* メッセージを受信する */
// 		log("Task A: before tk_rcv_mbf", NO_PARAM);
// 		tk_rcv_mbf(mbfidX, msg, TMO_FEVR);
// 		log("Task A: after  tk_rcv_mbf, msg[0]=", msg[0]);
// 	}

// 	log("Task A: exit and delete", NO_PARAM);
// 	tk_exd_tsk();			/* 自タスクを終了・削除 */
// }

// /* タスクB */
// void taskB(INT stacd, void *exinf)
// {
// 	INT i, k;
// 	UB msg[MSGLEN];

// 	log("Task B: start", NO_PARAM);

// 	for (i = 0; i < 5; i++) {

// 		/* メッセージを作成する */
// 		for (k = 0; k < MSGLEN; k++) msg[k] = (UB)(i + k);

// 		/* メッセージを送信する */
// 		log("Task B: before tk_snd_mbf, msg[0]=", msg[0]);
// 		tk_snd_mbf(mbfidX, msg, MSGLEN, TMO_FEVR);
// 		log("Task B: after  tk_snd_mbf", NO_PARAM);
// 	}

// 	log("Task B: exit and delete", NO_PARAM);
// 	tk_exd_tsk();			/* 自タスクを終了・削除 */
// }

// /* usermainによる初期化処理 */
// const T_CTSK ctskA = {0, (TA_HLNG | TA_RNG3), &taskA, 10, 1024, 0};
// const T_CTSK ctskB = {0, (TA_HLNG | TA_RNG3), &taskB, 20, 1024, 0};
// const T_CMBF cmbfX = {0, (TA_TFIFO | TA_MFIFO), MBFBUFSZ, MSGLEN};

// EXPORT INT usermain(void)
// {
// 	log("Start User-main program", NO_PARAM);	/* 実行開始を示すログ */

// 	tidA = tk_cre_tsk(&ctskA);		/* タスクAを生成 */
// 	tidB = tk_cre_tsk(&ctskB);		/* タスクBを生成 */
// 	mbfidX = tk_cre_mbf(&cmbfX);		/* メッセージバッファXを生成 */

// 	tk_sta_tsk(tidA, 0);			/* タスクAを起動 */
// 	tk_sta_tsk(tidB, 0);			/* タスクBを起動 */

// 	tk_dly_tsk(1000);			/* 終了後に1秒の時間待ち */
// 	log("End User-main program", NO_PARAM);	/* 実行終了のログ */
// 	print_log();				/* ログを表示 */

// 	tk_slp_tsk(TMO_FEVR);
// 	return 0;
// }

// receiver_main.c
// #include <tk/tkernel.h>
// #include <tm/tmonitor.h>
// #include "radio_driver.h"
// #include "radio_config.h"   // <-- Add this line

// EXPORT void usermain(void)
// {
//     tm_printf("=== Receiver STARTED ===\n");

//     radio_init();
//     tk_dly_tsk(100);  // Let radio settle

//     UB buffer[MICROBIT_RADIO_MAX_PACKET];
//     INT len;
//     INT loop_counter = 0;

//     while (1) {
//         len = radio_receive(buffer);
//         if (len > 0) {
//             tm_printf("Received: ");
//             for (int i = 0; i < len; i++) {
//                 tm_putchar(buffer[i]);
//             }
//             tm_putchar('\n');
//         } else {
//             tm_printf(".");  // Heartbeat dot
//         }
//     tk_dly_tsk(500);  // Delay for readability
//     }
// }




// receiver code
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <tk/syslib.h>
#include <stdint.h>              // Needed for uint32_t
#include "radio_driver.h"        // Exposes radio_init and radio_receive
#include "radio_config.h"        // Exposes MICROBIT_RADIO_MAX_PACKET

extern void RADIO_IRQHandler(void);  // our IRQ function

EXPORT void usermain(void)
{
    tm_printf("=== Receiver code  ===\n");

    // STEP 1: Register interrupt handler
    T_DINT dint;
    dint.intatr = TA_HLNG;
    dint.inthdr = (FP)RADIO_IRQHandler;
    tk_def_int(1, &dint);

    // STEP 2: Enable RADIO interrupt globally
    #define NVIC_ISER0  (*(volatile uint32_t *)0xE000E100)
    NVIC_ISER0 = (1 << 1);  // IRQ 1 = RADIO

    // Continue with radio setup
    radio_init();
    tk_dly_tsk(100);

    UB buffer[MICROBIT_RADIO_MAX_PACKET];
    RELTIM last_tick = 0;
    tk_get_tim(&last_tick);  // current system time in milliseconds

    while (1) {
        RELTIM now;
        tk_get_tim(&now);

     // Print every 4000 ms (4 seconds)
        if ((now - last_tick) >= 4000) {
            tm_printf("Waiting for packets...\n");
            last_tick = now;
        }


        INT len = radio_receive(buffer);
        if (len > 0) {
            tm_printf("Received: ");
            for (int i = 0; i < len; ++i) tm_putchar(buffer[i]);
            tm_putchar('\n');
        }

        tk_dly_tsk(500);
    }
}