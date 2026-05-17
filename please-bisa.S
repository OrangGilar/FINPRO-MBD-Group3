.section .bss
time1_ms:   .space 4
time2_ms:   .space 4
g_speed10:  .space 2
g_status:   .space 1

.section .text

.global etle_setup
etle_setup:
    in   r24, 0x0A
    andi r24, 0xF3
    out  0x0A, r24

    in   r24, 0x0B
    ori  r24, 0x0C
    out  0x0B, r24

    in   r24, 0x04
    ori  r24, 0x0F
    out  0x04, r24

    in   r24, 0x05
    andi r24, 0xF0
    out  0x05, r24

    clr  r24
    sts  g_status, r24
    sts  g_speed10+0, r24
    sts  g_speed10+1, r24

    rcall _yellow_on
    ret

.global etle_loop
etle_loop:
    push r14
    push r15
    push r16
    push r17
    push r28
    push r29

    clr  r24
    sts  g_status, r24
    sts  g_speed10+0, r24
    sts  g_speed10+1, r24

    rcall _yellow_on

_poll_s1:
    in   r24, 0x09
    andi r24, 0x04
    brne _poll_s1

    call millis
    sts  time1_ms+0, r22
    sts  time1_ms+1, r23
    sts  time1_ms+2, r24
    sts  time1_ms+3, r25

_release_s1:
    in   r24, 0x09
    andi r24, 0x04
    breq _release_s1

_poll_s2:
    in   r24, 0x09
    andi r24, 0x08
    brne _poll_s2

    call millis
    sts  time2_ms+0, r22
    sts  time2_ms+1, r23
    sts  time2_ms+2, r24
    sts  time2_ms+3, r25

    lds  r18, time1_ms+0
    lds  r19, time1_ms+1
    lds  r20, time1_ms+2
    lds  r21, time1_ms+3

    lds  r22, time2_ms+0
    lds  r23, time2_ms+1
    lds  r24, time2_ms+2
    lds  r25, time2_ms+3

    sub  r22, r18
    sbc  r23, r19
    sbc  r24, r20
    sbc  r25, r21

    tst  r25
    brne _check_overflow
    tst  r24
    brne _check_overflow
    tst  r23
    brne _check_overflow
    cpi  r22, 10
    brsh _check_overflow
    rjmp _exit_loop

_check_overflow:
    tst  r24
    brne _set_safe
    tst  r25
    brne _set_safe

    ldi  r26, 0x20
    ldi  r27, 0x1C
    rcall _div16u
    
    sts  g_speed10+0, r18
    sts  g_speed10+1, r19

    tst  r19
    brne _set_overspeed
    cpi  r18, 50
    brsh _set_overspeed

_set_safe:
    ldi  r24, 1
    sts  g_status, r24
    rcall _green_on
    rcall _buzzer_off
    rjmp _exit_loop

_set_overspeed:
    ldi  r24, 2
    sts  g_status, r24
    rcall _red_on
    rcall _buzzer_on

_exit_loop:
    pop  r29
    pop  r28
    pop  r17
    pop  r16
    pop  r15
    pop  r14
    ret

.global etle_get_status
etle_get_status:
    lds  r24, g_status
    ret

.global etle_get_speed
etle_get_speed:
    lds  r24, g_speed10+0
    lds  r25, g_speed10+1
    ret

_all_off:
    in   r24, 0x05
    andi r24, 0xF0
    out  0x05, r24
    ret

_red_on:
    rcall _all_off
    in   r24, 0x05
    ori  r24, 0x08
    out  0x05, r24
    ret

_yellow_on:
    rcall _all_off
    in   r24, 0x05
    ori  r24, 0x04
    out  0x05, r24
    ret

_green_on:
    rcall _all_off
    in   r24, 0x05
    ori  r24, 0x02
    out  0x05, r24
    ret

_buzzer_on:
    in   r24, 0x05
    ori  r24, 0x01
    out  0x05, r24
    ret

_buzzer_off:
    in   r24, 0x05
    andi r24, 0xFE
    out  0x05, r24
    ret

_div16u:
    clr  r18
    clr  r19
    clr  r16
    clr  r17
    ldi  r24, 16

_div16u_loop:
    lsl  r26
    rol  r27
    rol  r16
    rol  r17

    sub  r16, r22
    sbc  r17, r23
    brsh _div16u_norestore

    add  r16, r22
    adc  r17, r23
    clc
    rjmp _div16u_next

_div16u_norestore:
    sec

_div16u_next:
    rol  r18
    rol  r19
    dec  r24
    brne _div16u_loop
    ret