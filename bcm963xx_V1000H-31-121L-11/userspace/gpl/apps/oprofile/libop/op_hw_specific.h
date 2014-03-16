/* 
 * @file architecture specific interfaces
 * @remark Copyright 2008 Intel Corporation
 * @remark Read the file COPYING
 * @author Andi Kleen
 */

#if defined(__i386__) || defined(__x86_64__) 

/* Assume we run on the same host as the profilee */

#define num_to_mask(x) ((1U << (x)) - 1)

static inline int cpuid_vendor(char *vnd)
{
	union {
		struct {
			unsigned b,d,c;
		};
		char v[12];
	} v;
	unsigned eax;
	asm("cpuid" : "=a" (eax), "=b" (v.b), "=c" (v.c), "=d" (v.d) : "0" (0));
	return !strncmp(v.v, vnd, 12);
}

/* Work around Nehalem spec update AAJ79: CPUID incorrectly indicates
   unhalted reference cycle architectural event is supported. We assume
   steppings after C0 report correct data in CPUID. */
static inline void workaround_nehalem_aaj79(unsigned *ebx)
{
	union {
		unsigned eax;
		struct {
			unsigned stepping : 4;
			unsigned model : 4;
			unsigned family : 4;
			unsigned type : 2;
			unsigned res : 2;
			unsigned ext_model : 4;
			unsigned ext_family : 8;
			unsigned res2 : 4;
		};
	} v;
	unsigned model;

	if (!cpuid_vendor("GenuineIntel"))
		return;
	asm("cpuid" : "=a" (v.eax) : "0" (1) : "ecx","ebx","edx");
	model = (v.ext_model << 4) + v.model;
	if (v.family != 6 || model != 26 || v.stepping > 4)
		return;
	*ebx |= (1 << 2);	/* disable unsupported event */
}

static inline unsigned arch_get_filter(op_cpu cpu_type)
{
	if (cpu_type == CPU_ARCH_PERFMON) { 
		unsigned ebx, eax;
		asm("cpuid" : "=a" (eax), "=b" (ebx) : "0" (0xa) : "ecx","edx");
		workaround_nehalem_aaj79(&ebx);
		return ebx & num_to_mask(eax >> 24);
	}
	return -1U;
}

static inline int arch_num_counters(op_cpu cpu_type) 
{
	if (cpu_type == CPU_ARCH_PERFMON) {
		unsigned v;
		asm("cpuid" : "=a" (v) : "0" (0xa) : "ebx","ecx","edx");
		return (v >> 8) & 0xff;
	} 
	return -1;
}

static inline unsigned arch_get_counter_mask(void)
{
	unsigned v;
	asm("cpuid" : "=a" (v) : "0" (0xa) : "ebx","ecx","edx");
	return num_to_mask((v >> 8) & 0xff);	
}

#else

static inline unsigned arch_get_filter(op_cpu cpu_type)
{
	/* Do something with passed arg to shut up the compiler warning */
	if (cpu_type != CPU_NO_GOOD)
		return 0;
	return 0;
}

static inline int arch_num_counters(op_cpu cpu_type) 
{
	/* Do something with passed arg to shut up the compiler warning */
	if (cpu_type != CPU_NO_GOOD)
		return -1;
	return -1;
}

static inline unsigned arch_get_counter_mask(void)
{
	return 0;
}

#endif
