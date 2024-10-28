.code
d_jdVM proc
      push   rdx
      push   rcx
      push   rbx

      mov    rax, 'VMXh'
      mov    rbx, 0     ; any value but not the MAGIC VALUE
      mov    rcx, 10    ; get VMWare version
      mov    rdx, 'VX'  ; port number

      in     rax, dx    ; read port
                        ; on return EAX returns the VERSION
      cmp    rbx, 'VMXh'; is it a reply from VMWare?
      setz   al         ; set return value
      movzx rax,al

      pop    rbx
      pop    rcx
      pop    rdx

      ret
d_jdVM endp
end

;https://learn.microsoft.com/zh-cn/cpp/assembler/masm/masm-for-x64-ml64-exe?view=msvc-170
;堆溢出触发异常导致系统进入SEH(?)