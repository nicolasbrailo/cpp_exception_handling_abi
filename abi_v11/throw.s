	.file	"throw.cpp"
	.text
	.globl	_Z5raisev
	.type	_Z5raisev, @function
_Z5raisev:
.LFB0:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$24, %esp
	movl	$1, (%esp)
	call	__cxa_allocate_exception
	movl	$0, 8(%esp)
	movl	$_ZTI9Exception, 4(%esp)
	movl	%eax, (%esp)
	call	__cxa_throw
	.cfi_endproc
.LFE0:
	.size	_Z5raisev, .-_Z5raisev
	.section	.rodata
	.align 4
.LC0:
	.string	"Running a try which will never throw."
	.align 4
.LC1:
	.string	"try_but_dont_catch handled the exception"
	.align 4
.LC2:
	.string	"Exception caught... with the wrong catch!"
.LC3:
	.string	"Caught a Fake_Exception!"
	.text
	.globl	_Z18try_but_dont_catchv
	.type	_Z18try_but_dont_catchv, @function
_Z18try_but_dont_catchv:
.LFB1:
	.cfi_startproc
	.cfi_personality 0,__gxx_personality_v0
	.cfi_lsda 0,.LLSDA1
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$36, %esp
	.cfi_offset 3, -12
	movl	$.LC0, (%esp)
.LEHB0:
	call	puts
.LEHE0:
.L7:
.LEHB1:
	call	_Z5raisev
.LEHE1:
.L12:
	movl	$.LC1, (%esp)
.LEHB2:
	call	puts
	jmp	.L17
.L14:
	movl	%eax, %ebx
	call	__cxa_end_catch
	movl	%ebx, %eax
	movl	%eax, (%esp)
	call	_Unwind_Resume
.L13:
	cmpl	$1, %edx
	je	.L6
	movl	%eax, (%esp)
	call	_Unwind_Resume
.LEHE2:
.L6:
	movl	%eax, (%esp)
	call	__cxa_begin_catch
	movl	%eax, -12(%ebp)
	movl	$.LC2, (%esp)
.LEHB3:
	call	puts
.LEHE3:
	call	__cxa_end_catch
	jmp	.L7
.L16:
	movl	%eax, %ebx
	call	__cxa_end_catch
	movl	%ebx, %eax
	movl	%eax, (%esp)
.LEHB4:
	call	_Unwind_Resume
.L15:
	cmpl	$1, %edx
	je	.L11
	movl	%eax, (%esp)
	call	_Unwind_Resume
.LEHE4:
.L11:
	movl	%eax, (%esp)
	call	__cxa_begin_catch
	movl	%eax, -16(%ebp)
	movl	$.LC3, (%esp)
.LEHB5:
	call	puts
.LEHE5:
	call	__cxa_end_catch
	jmp	.L12
.L17:
	addl	$36, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE1:
	.globl	__gxx_personality_v0
	.section	.gcc_except_table,"a",@progbits
	.align 4
.LLSDA1:
	.byte	0xff
	.byte	0
	.uleb128 .LLSDATT1-.LLSDATTD1
.LLSDATTD1:
	.byte	0x1
	.uleb128 .LLSDACSE1-.LLSDACSB1
.LLSDACSB1:
	.uleb128 .LEHB0-.LFB1
	.uleb128 .LEHE0-.LEHB0
	.uleb128 .L13-.LFB1
	.uleb128 0x1
	.uleb128 .LEHB1-.LFB1
	.uleb128 .LEHE1-.LEHB1
	.uleb128 .L15-.LFB1
	.uleb128 0x1
	.uleb128 .LEHB2-.LFB1
	.uleb128 .LEHE2-.LEHB2
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB3-.LFB1
	.uleb128 .LEHE3-.LEHB3
	.uleb128 .L14-.LFB1
	.uleb128 0
	.uleb128 .LEHB4-.LFB1
	.uleb128 .LEHE4-.LEHB4
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB5-.LFB1
	.uleb128 .LEHE5-.LEHB5
	.uleb128 .L16-.LFB1
	.uleb128 0
.LLSDACSE1:
	.byte	0x1
	.byte	0
	.align 4
	.long	_ZTI14Fake_Exception
.LLSDATT1:
	.text
	.size	_Z18try_but_dont_catchv, .-_Z18try_but_dont_catchv
	.section	.rodata
.LC4:
	.string	"catchit handled the exception"
.LC5:
	.string	"Caught an Exception!"
	.text
	.globl	_Z7catchitv
	.type	_Z7catchitv, @function
_Z7catchitv:
.LFB2:
	.cfi_startproc
	.cfi_personality 0,__gxx_personality_v0
	.cfi_lsda 0,.LLSDA2
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	pushl	%ebx
	subl	$36, %esp
	.cfi_offset 3, -12
.LEHB6:
	call	_Z18try_but_dont_catchv
.LEHE6:
.L25:
	movl	$.LC4, (%esp)
.LEHB7:
	call	puts
	jmp	.L29
.L27:
	movl	%eax, %ebx
	call	__cxa_end_catch
	movl	%ebx, %eax
	movl	%eax, (%esp)
	call	_Unwind_Resume
.L28:
	movl	%eax, %ebx
	call	__cxa_end_catch
	movl	%ebx, %eax
	movl	%eax, (%esp)
	call	_Unwind_Resume
.L26:
	cmpl	$1, %edx
	je	.L23
	cmpl	$2, %edx
	je	.L24
	movl	%eax, (%esp)
	call	_Unwind_Resume
.LEHE7:
.L23:
	movl	%eax, (%esp)
	call	__cxa_begin_catch
	movl	%eax, -12(%ebp)
	movl	$.LC3, (%esp)
.LEHB8:
	call	puts
.LEHE8:
	call	__cxa_end_catch
	jmp	.L25
.L24:
	movl	%eax, (%esp)
	call	__cxa_begin_catch
	movl	%eax, -16(%ebp)
	movl	$.LC5, (%esp)
.LEHB9:
	call	puts
.LEHE9:
	call	__cxa_end_catch
	jmp	.L25
.L29:
	addl	$36, %esp
	popl	%ebx
	.cfi_restore 3
	popl	%ebp
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE2:
	.section	.gcc_except_table
	.align 4
.LLSDA2:
	.byte	0xff
	.byte	0
	.uleb128 .LLSDATT2-.LLSDATTD2
.LLSDATTD2:
	.byte	0x1
	.uleb128 .LLSDACSE2-.LLSDACSB2
.LLSDACSB2:
	.uleb128 .LEHB6-.LFB2
	.uleb128 .LEHE6-.LEHB6
	.uleb128 .L26-.LFB2
	.uleb128 0x3
	.uleb128 .LEHB7-.LFB2
	.uleb128 .LEHE7-.LEHB7
	.uleb128 0
	.uleb128 0
	.uleb128 .LEHB8-.LFB2
	.uleb128 .LEHE8-.LEHB8
	.uleb128 .L27-.LFB2
	.uleb128 0
	.uleb128 .LEHB9-.LFB2
	.uleb128 .LEHE9-.LEHB9
	.uleb128 .L28-.LFB2
	.uleb128 0
.LLSDACSE2:
	.byte	0x2
	.byte	0
	.byte	0x1
	.byte	0x7d
	.align 4
	.long	_ZTI9Exception
	.long	_ZTI14Fake_Exception
.LLSDATT2:
	.text
	.size	_Z7catchitv, .-_Z7catchitv
	.globl	seppuku
	.type	seppuku, @function
seppuku:
.LFB3:
	.cfi_startproc
	pushl	%ebp
	.cfi_def_cfa_offset 8
	.cfi_offset 5, -8
	movl	%esp, %ebp
	.cfi_def_cfa_register 5
	subl	$8, %esp
	call	_Z7catchitv
	leave
	.cfi_restore 5
	.cfi_def_cfa 4, 4
	ret
	.cfi_endproc
.LFE3:
	.size	seppuku, .-seppuku
	.weak	_ZTS14Fake_Exception
	.section	.rodata._ZTS14Fake_Exception,"aG",@progbits,_ZTS14Fake_Exception,comdat
	.type	_ZTS14Fake_Exception, @object
	.size	_ZTS14Fake_Exception, 17
_ZTS14Fake_Exception:
	.string	"14Fake_Exception"
	.weak	_ZTI14Fake_Exception
	.section	.rodata._ZTI14Fake_Exception,"aG",@progbits,_ZTI14Fake_Exception,comdat
	.align 4
	.type	_ZTI14Fake_Exception, @object
	.size	_ZTI14Fake_Exception, 8
_ZTI14Fake_Exception:
	.long	_ZTVN10__cxxabiv117__class_type_infoE+8
	.long	_ZTS14Fake_Exception
	.weak	_ZTS9Exception
	.section	.rodata._ZTS9Exception,"aG",@progbits,_ZTS9Exception,comdat
	.type	_ZTS9Exception, @object
	.size	_ZTS9Exception, 11
_ZTS9Exception:
	.string	"9Exception"
	.weak	_ZTI9Exception
	.section	.rodata._ZTI9Exception,"aG",@progbits,_ZTI9Exception,comdat
	.align 4
	.type	_ZTI9Exception, @object
	.size	_ZTI9Exception, 8
_ZTI9Exception:
	.long	_ZTVN10__cxxabiv117__class_type_infoE+8
	.long	_ZTS9Exception
	.ident	"GCC: (Debian 4.7.1-7) 4.7.1"
	.section	.note.GNU-stack,"",@progbits
