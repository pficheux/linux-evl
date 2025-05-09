/* SPDX-License-Identifier: GPL-2.0 */
#ifndef __ASM_ARM_IRQFLAGS_H
#define __ASM_ARM_IRQFLAGS_H

#ifdef __KERNEL__

#include <asm/ptrace.h>
#include <asm/barrier.h>

/*
 * CPU interrupt mask handling.
 */
#ifdef CONFIG_CPU_V7M
#define IRQMASK_REG_NAME_R "primask"
#define IRQMASK_REG_NAME_W "primask"
#define IRQMASK_I_BIT	1
#define IRQMASK_I_POS	0
#else
#define IRQMASK_REG_NAME_R "cpsr"
#define IRQMASK_REG_NAME_W "cpsr_c"
#define IRQMASK_I_BIT	PSR_I_BIT
#define IRQMASK_I_POS	7
#endif
#define IRQMASK_i_POS	31

#if __LINUX_ARM_ARCH__ >= 6

#define arch_local_irq_save arch_local_irq_save
static inline unsigned long native_irq_save(void)
{
	unsigned long flags;

	asm volatile(
		"	mrs	%0, " IRQMASK_REG_NAME_R "	@ native_irq_save\n"
		"	cpsid	i"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}

#define arch_local_irq_enable arch_local_irq_enable
static inline void native_irq_enable(void)
{
	asm volatile(
		"	cpsie i			@ native_irq_enable"
		:
		:
		: "memory", "cc");
}

#define arch_local_irq_disable arch_local_irq_disable
static inline void native_irq_disable(void)
{
	asm volatile(
		"	cpsid i			@ native_irq_disable"
		:
		:
		: "memory", "cc");
}

#define local_fiq_enable()  __asm__("cpsie f	@ __stf" : : : "memory", "cc")
#define local_fiq_disable() __asm__("cpsid f	@ __clf" : : : "memory", "cc")

#ifndef CONFIG_CPU_V7M
#define local_abt_enable()  __asm__("cpsie a	@ __sta" : : : "memory", "cc")
#define local_abt_disable() __asm__("cpsid a	@ __cla" : : : "memory", "cc")
#else
#define local_abt_enable()	do { } while (0)
#define local_abt_disable()	do { } while (0)
#endif
#else

/*
 * Save the current interrupt enable state & disable IRQs
 */
#define arch_local_irq_save arch_local_irq_save
static inline unsigned long native_irq_save(void)
{
	unsigned long flags, temp;

	asm volatile(
		"	mrs	%0, cpsr	@ native_irq_save\n"
		"	orr	%1, %0, #128\n"
		"	msr	cpsr_c, %1"
		: "=r" (flags), "=r" (temp)
		:
		: "memory", "cc");
	return flags;
}

/*
 * Enable IRQs
 */
#define arch_local_irq_enable arch_local_irq_enable
static inline void native_irq_enable(void)
{
	unsigned long temp;
	asm volatile(
		"	mrs	%0, cpsr	@ native_irq_enable\n"
		"	bic	%0, %0, #128\n"
		"	msr	cpsr_c, %0"
		: "=r" (temp)
		:
		: "memory", "cc");
}

/*
 * Disable IRQs
 */
#define arch_local_irq_disable arch_local_irq_disable
static inline void native_irq_disable(void)
{
	unsigned long temp;
	asm volatile(
		"	mrs	%0, cpsr	@ native_irq_disable\n"
		"	orr	%0, %0, #128\n"
		"	msr	cpsr_c, %0"
		: "=r" (temp)
		:
		: "memory", "cc");
}

/*
 * Enable FIQs
 */
#define local_fiq_enable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ stf\n"		\
"	bic	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

/*
 * Disable FIQs
 */
#define local_fiq_disable()					\
	({							\
		unsigned long temp;				\
	__asm__ __volatile__(					\
	"mrs	%0, cpsr		@ clf\n"		\
"	orr	%0, %0, #64\n"					\
"	msr	cpsr_c, %0"					\
	: "=r" (temp)						\
	:							\
	: "memory", "cc");					\
	})

#define local_abt_enable()	do { } while (0)
#define local_abt_disable()	do { } while (0)
#endif

static inline void native_irq_sync(void)
{
	native_irq_enable();
	isb();
	native_irq_disable();
}

/*
 * Save the current interrupt enable state.
 */
#define arch_local_save_flags arch_local_save_flags
static inline unsigned long native_save_flags(void)
{
	unsigned long flags;
	asm volatile(
		"	mrs	%0, " IRQMASK_REG_NAME_R "	@ native_save_flags"
		: "=r" (flags) : : "memory", "cc");
	return flags;
}

/*
 * restore saved IRQ state
 */
#define arch_local_irq_restore arch_local_irq_restore
static inline void native_irq_restore(unsigned long flags)
{
	unsigned long temp = 0;
	flags &= ~(1 << 6);
	asm volatile (
		" mrs %0, cpsr"
		: "=r" (temp)
		:
		: "memory", "cc");
		/* Preserve FIQ bit */
		temp &= (1 << 6);
		flags = flags | temp;
	asm volatile(
		"	msr	" IRQMASK_REG_NAME_W ", %0	@ native_irq_restore"
		:
		: "r" (flags)
		: "memory", "cc");
}

#define arch_irqs_disabled_flags arch_irqs_disabled_flags
static inline int native_irqs_disabled_flags(unsigned long flags)
{
	return flags & IRQMASK_I_BIT;
}

static inline bool native_irqs_disabled(void)
{
	unsigned long flags = native_save_flags();
	return native_irqs_disabled_flags(flags);
}

#include <asm/irq_pipeline.h>
#include <asm-generic/irqflags.h>

#endif /* ifdef __KERNEL__ */
#endif /* ifndef __ASM_ARM_IRQFLAGS_H */
