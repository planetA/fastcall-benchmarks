#ifdef __aarch64__

#define SVC_IMM 0xFC

.text

.global invoke_fastcall
invoke_fastcall:
  mov  x8, x0
  mov  x0, x1
  mov  x1, x2
  mov  x2, x3
  mov  x3, x4
  mov  x4, x5
  mov  x5, x6
  mov  x6, x7
  svc  #SVC_IMM
  ret

#endif /* __aarch64__ */
