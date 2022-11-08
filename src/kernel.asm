extern ready_q, dequeue, running, enqueue
global k_printstr, go, dispatch_leave, dispatch, lidtr

; print chars to screen until a null char (zero byte) is reached
k_printstr:

    push ebp ; save the functions stack frame
    mov ebp, esp ; ebp = esp
    pushf
    push eax
    push esi
    push edi
    push edx
    
    mov eax, [ebp + 4 + 4 + 4] ; eax = row
    mov edi, 80 ; edi = 80
    mul edi ; eax = row * 80
    add eax, [ebp + 4 + 4 + 4 + 4] ; eax = (row * 80 + col)
    mov edi, 2 ; edi = 2
    mul edi ; eax = (row * 80 + col) * 2
    mov edi, 0xb8000 ; edi = 0xb8000
    add edi, eax ; edi = ((row * 80 + col) * 2) + 0xb8000 (this is the memory location to start writing string passed to the function)

    mov esi, [ebp + 4 + 4]

    loop_begin:
    
    cmp BYTE [esi], 0 ; check for reaching zero terminating character for string
    je loop_end
    cmp edi, 0xb8f9e ; check to see if we have reached video memory
    jg loop_end
    movsb ; move the character to video memory, increment video memory and string pointer
    mov BYTE [edi], 15 ; move color code to video memory
    add edi, 1
    jmp loop_begin

    loop_end:
    pop edx
    pop edi
    pop esi
    pop eax
    popf
    pop ebp

    ret ; return to the calling function

go:
    push DWORD [ready_q] ; push the ready_q onto the stack
    call dequeue ; call dequeue to get pcb
    mov [running], eax  ; pop the returned value in to the 32-bit running register

go_rest:
    mov esp, [eax]
    popad
    ret

dispatch:
    call yield
dispatch_leave:
    iret
yield:
    pushad
    mov ebx, [running] 
    mov [ebx], esp
    push DWORD [running] ; running pcb pointer on stack
    push DWORD [ready_q] ; ready_q address on stack
    call enqueue
    jmp go
    ret ; should never get here

lidtr:
    push ebp ; save the functions stack frame
    mov ebp, esp ; ebp = esp
    pushf
    push eax
    mov eax, [ebp + 4 + 4]
    lidt [eax]
    pop eax
    popf
    pop ebp
    ret
