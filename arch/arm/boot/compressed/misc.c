/*
 * misc.c
 * 
 * This is a collection of several routines from gzip-1.0.3 
 * adapted for Linux.
 *
 * malloc by Hannu Savolainen 1993 and Matthias Urlichs 1994
 *
 * Modified for ARM Linux by Russell King
 *
 * Nicolas Pitre <nico@visuaide.com>  1999/04/14 :
 *  For this code to run directly from Flash, all constant variables must
 *  be marked with 'const' and all other variables initialized at run-time 
 *  only.  This way all non constant variables will end up in the bss segment,
 *  which should point to addresses in RAM and cleared to 0 on start.
 *  This allows for a much quicker boot time.
 */

//#include <stdint.h>
unsigned int __machine_arch_type;

#include <linux/compiler.h>	/* for inline */
#include <linux/types.h>
#include <linux/linkage.h>
#include <asm/string.h>

static void putstr(const char *ptr);
extern void error(char *x);

#include CONFIG_UNCOMPRESS_INCLUDE

#ifdef CONFIG_DEBUG_ICEDCC

#if defined(CONFIG_CPU_V6) || defined(CONFIG_CPU_V6K) || defined(CONFIG_CPU_V7)

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c0, c1, 0" : "=r" (status));
	} while (status & (1 << 29));

	asm("mcr p14, 0, %0, c0, c5, 0" : : "r" (ch));
}


#elif defined(CONFIG_CPU_XSCALE)

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c14, c0, 0" : "=r" (status));
	} while (status & (1 << 28));

	asm("mcr p14, 0, %0, c8, c0, 0" : : "r" (ch));
}

#else

static void icedcc_putc(int ch)
{
	int status, i = 0x4000000;

	do {
		if (--i < 0)
			return;

		asm volatile ("mrc p14, 0, %0, c0, c0, 0" : "=r" (status));
	} while (status & 2);

	asm("mcr p14, 0, %0, c1, c0, 0" : : "r" (ch));
}

#endif

#define putc(ch)	icedcc_putc(ch)
#endif

static void putstr(const char *ptr)
{
	char c;

	while ((c = *ptr++) != '\0') {
		if (c == '\n')
			putc('\r');
		putc(c);
	}

	flush();
}

/*
 * gzip declarations
 */
extern char input_data[];
extern char input_data_end[];

unsigned char *output_data;

unsigned long free_mem_ptr;
unsigned long free_mem_end_ptr;

#ifndef arch_error
#define arch_error(x)
#endif

void error(char *x)
{
	arch_error(x);

	putstr("\n\n");
	putstr(x);
	putstr("\n\n -- System halted");

	while(1);	/* Halt */
}

void puthex(unsigned long val){

	char x[9];
	unsigned char i, c;
	
	x[8] = 0;
	
	for(i = 0; i < 8; i++){
		
		c = val & 0x0F;
		val >>= 4;
		c = (c >= 10) ? (c + 'A' - 10) : (c + '0');
		x[7 - i] = c;	
	}
	
	putstr(x);
}

void putdec(unsigned long val){
	
	char x[16];
	unsigned char i, c;
	
	x[sizeof(x) - 1] = 0;
	
	for(i = 0; i < sizeof(x) - 1; i++){
		
		c = (val % 10) + '0';
		val /= 10;
		x[sizeof(x) - 2 - i] = c;	
		if(!val) break;
	}

	putstr(x + sizeof(x) - 2 - i);
}

asmlinkage void __div0(void)
{
	error("Attempting division by 0!");
}

extern int do_decompress(u8 *input, int len, u8 *output, void (*error)(char *x));


void
decompress_kernel(unsigned long output_start, unsigned long free_mem_ptr_p,
		unsigned long free_mem_ptr_end_p,
		int arch_id)
{
	//int ret;

	output_data		= (unsigned char *)output_start;
	free_mem_ptr		= free_mem_ptr_p;
	free_mem_end_ptr	= free_mem_ptr_end_p;
	__machine_arch_type	= arch_id;

	arch_decomp_setup();

    putstr("Found Linux @ 0x");
    puthex(input_data);
    putstr(" (");
    putdec(input_data_end - input_data);
    putstr(" bytes). Copying to 0x");
    puthex(output_data);

	putstr("\nCopying Linux...");
	
	/*
	 * You can see that this is the function that does the decompression process
	 * It receive the location of the compressed kernel, the length of the data, the address to output the decompressed thing, and a callback function for error handling (which we don't need in our case)
	 * Let's just copy the uncompressed kernel to output_data location using memcpy 
	 */
	//ret = do_decompress(input_data, input_data_end - input_data,
	//		    output_data, error);

	memcpy(output_data, input_data, input_data_end - input_data);

	// memcpy function is defined in <asm/string.h>, so we need to include that header

	//if (ret)
	//	error("decompressor returned an error");
	//else
	putstr(" done, booting the kernel.\n");
}
