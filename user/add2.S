.section .data
err_msg: .asciz "incorrect input format\n"
out_fmt: .asciz "%d\n"
debug_fmt: .asciz "|%s|\n"

.section .bss
buffer: .space 1025

.section .text
.globl main
main:
    li t1, 0                    # i
    li t2, 0                    # flagErr
    li t3, 0                    # flagEnd
    li t4, 0                    # flagSpace
    li t5, 0                    # idxSpace
    li t6, 1024
    la t0, buffer               # t0 = &buf[0]
    la a0, buffer               # a0 = &buf[0]
    li a1, 1024
    call gets

loop:
    bge t1, t6, check_format    # if i > 1024
    lbu a0, 0(t0)               # a0 = *t0
    beqz a0, check_format       # if a0 == 0
    li a1, '\n'
    beq a0, a1, handle_newline  # if a0 == '\n'
    li a1, '\r'                 
    beq a0, a1, handle_newline  # if a0 == '\r'
    li a1, ' '
    bne a0, a1, check_digit     # if a0 != ' '
    bnez t4, set_error          # if flagSpace
    li t4, 1                    # flagSpace = 1
    mv t5, t1                   # idxSpace = i
    j next_char
    
check_digit:
    li a1, '0'
    blt a0, a1, set_error       # if a0 < '0'
    li a1, '9'
    bgt a0, a1, set_error       # if a0 > '9'
    j next_char
    
set_error:
    li t2, 1                    # flagErr = 1
    
next_char:
    addi t0, t0, 1              # t0++
    addi t1, t1, 1              # i++
    j loop

handle_newline:
    li t3, 1                    # flagEnd = 1
    sb zero, 0(t0)              # *s = '\0'
    
check_format:
    la a0, debug_fmt
    la a1, buffer               # a1 = &buf[0]
    call printf
    beqz t4, format_error       # if !flagSpace
    beqz t5, format_error       # if !idxSpace
    addi a0, t5, 1              
    beq a0, t1, format_error    # if idxSpace + 1 == i
    bnez t2, format_error       # if flagErr
    la t0, buffer
    add t0, t0, t5              # t0 = &buf[idxSpace]
    sb zero, 0(t0)              # buf[idxSpace] = 0
    beqz t3, end_null           # if !flagEnd
    j call_atoi
    
end_null:
    la t0, buffer               
    addi t0, t0, 1024           # t0 = &buf[1024]
    sb zero, 0(t0)              # buf[1024] = 0
    
call_atoi:
    la a0, buffer               # a0 = &buf[0]
    call atoi
    mv s0, a0                   # s0 = a
    la t0, buffer
    add t0, t0, t5              # t0 = &buf[idxSpace]
    addi a0, t0, 1              # a0 = &buf[idxSpace + 1]
    call atoi
    mv s1, a0                   # s1 = b
    add s2, s0, s1              # s2 = a + b
    la a0, out_fmt
    mv a1, s2
    call printf
    li a0, 0
    call exit

format_error:
    li a0, 2
    la a1, err_msg
    call fprintf
    li a0, 1
    call exit
