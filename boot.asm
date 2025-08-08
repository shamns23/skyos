; محمل إقلاع بسيط لنظام تشغيل
bits 16                     ; وضع 16 بت
org 0x7c00                  ; نقطة بداية التنفيذ

; ثوابت
KERNEL_OFFSET equ 0x1000    ; عنوان تحميل النواة

; حفظ رقم القرص الذي تم الإقلاع منه
mov [BOOT_DRIVE], dl

; إعداد المكدس
mov bp, 0x9000
mov sp, bp

; رسالة ترحيب
mov si, MSG_REAL_MODE
call print_string

; تحميل النواة
call load_kernel

; الانتقال إلى الوضع المحمي 32 بت
call switch_to_pm

jmp $                      ; حلقة لا نهائية

; دالة لتحميل النواة من القرص
load_kernel:
    mov si, MSG_LOAD_KERNEL
    call print_string
    
    mov bx, KERNEL_OFFSET   ; عنوان التحميل
    mov dh, 15              ; عدد القطاعات للقراءة
    mov dl, [BOOT_DRIVE]    ; رقم القرص
    call disk_load
    ret

; دالة لقراءة قطاعات من القرص
disk_load:
    push dx
    
    mov ah, 0x02            ; وظيفة BIOS: قراءة قطاعات
    mov al, dh              ; عدد القطاعات
    mov ch, 0x00            ; رقم الأسطوانة
    mov dh, 0x00            ; رقم الرأس
    mov cl, 0x02            ; رقم القطاع (يبدأ من 2، لأن 1 هو محمل الإقلاع)
    
    int 0x13                ; استدعاء BIOS
    
    jc disk_error           ; إذا حدث خطأ (CF=1)
    
    pop dx
    cmp dh, al              ; مقارنة عدد القطاعات المطلوبة مع المقروءة
    jne disk_error
    ret
    
disk_error:
    mov si, MSG_DISK_ERROR
    call print_string
    jmp $

; دالة لطباعة سلسلة نصية
print_string:
    pusha
    mov ah, 0x0e            ; وظيفة BIOS: طباعة حرف
.loop:
    lodsb                   ; تحميل حرف من SI إلى AL
    or al, al               ; اختبار إذا كان AL = 0 (نهاية السلسلة)
    jz .done
    int 0x10                ; استدعاء BIOS لطباعة الحرف
    jmp .loop
.done:
    popa
    ret

; الانتقال إلى الوضع المحمي
switch_to_pm:
    cli                     ; إيقاف المقاطعات
    lgdt [gdt_descriptor]   ; تحميل GDT
    
    ; تفعيل الوضع المحمي
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    
    ; القفز إلى الكود 32 بت
    jmp CODE_SEG:init_pm

; ثوابت وبيانات
BOOT_DRIVE db 0
MSG_REAL_MODE db "Started in 16-bit Real Mode", 0
MSG_LOAD_KERNEL db "Loading kernel into memory", 0
MSG_DISK_ERROR db "Disk read error!", 0
MSG_PROT_MODE db "Successfully landed in 32-bit Protected Mode", 0

; GDT (جدول الوصف العام)
gdt_start:

gdt_null:                   ; قطاع فارغ إلزامي
    dd 0x0
    dd 0x0

gdt_code:                   ; قطاع الشيفرة
    dw 0xffff               ; الحد (0-15)
    dw 0x0                  ; القاعدة (0-15)
    db 0x0                  ; القاعدة (16-23)
    db 10011010b            ; الوصول: حاضر=1, مستوى امتياز=00, نوع=1, نوع=1010 (تنفيذ/قراءة)
    db 11001111b            ; الحد (16-19) + علامات
    db 0x0                  ; القاعدة (24-31)

gdt_data:                   ; قطاع البيانات
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b            ; الوصول: حاضر=1, مستوى امتياز=00, نوع=1, نوع=0010 (قراءة/كتابة)
    db 11001111b
    db 0x0

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; حجم GDT
    dd gdt_start                ; عنوان GDT

; ثوابت للقطاعات
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; كود 32 بت
bits 32

init_pm:
    ; تحديث القطاعات
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    ; تحديث المكدس
    mov ebp, 0x90000
    mov esp, ebp
    
    ; القفز إلى النواة
    call KERNEL_OFFSET
    
    jmp $

; ملء القطاع وإضافة توقيع الإقلاع
times 510 - ($ - $$) db 0
dw 0xaa55