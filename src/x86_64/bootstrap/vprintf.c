/* vsprintf.c -- Lars Wirzenius & Linus Torvalds. */
/*
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
 */
#include "common.h"
#include "vprintf.h"
#include "lib.h"
#include "console.h"

BOOTSTRAP_CODE

/* we use this so that we can do without the ctype library */
#define is_digit(c)     ((c) >= '0' && (c) <= '9')

static int skip_atoi(const char **s)
{
	int i=0;

	while (is_digit(**s))
		i = i*10 + *((*s)++) - '0';
		
	return i;
}

#define ZEROPAD 1 /* pad with zero */
#define SIGN 2 /* unsigned/signed long */
#define PLUS 4 /* show plus */
#define SPACE 8 /* space if plus */
#define LEFT 16 /* left justified */
#define SPECIAL 32 /* 0x */
#define SMALL 64 /* use 'abcdef' instead of 'ABCDEF' */

#define do_div(n,base) ({ \
	int __res; \
	__asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
	__res; })

static void number(int num, int base, int size, int precision, int type)
{
	char c, sign, tmp[36];
	const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	if (type&SMALL) digits="0123456789abcdefghijklmnopqrstuvwxyz";
	
	if (type&LEFT) type &= ~ZEROPAD;
	
	if (base<2 || base>36)
			return;
	
	c = (type & ZEROPAD) ? '0' : ' ' ;
	
	if (type&SIGN && num<0) {
			sign='-';
			num = -num;
	} else
			sign=(type&PLUS) ? '+' : ((type&SPACE) ? ' ' : 0);
	
	if (sign) size--;
	
	if (type&SPECIAL) {
			if (base==16) size -= 2;
			else if (base==8) size--;
	}
	
	i=0;
	
	if (num==0)
			tmp[i++]='0';
	else while (num!=0)
			tmp[i++]=digits[do_div(num,base)];
	
	if (i>precision) precision=i;
	
	size -= precision;
	
	if (!(type&(ZEROPAD+LEFT)))
			while(size-->0)
					console_putc(' ');
	
	if (sign)
			console_putc(sign);
	
	if (type&SPECIAL) {
			if (base==8)
					console_putc('0');
			else if (base==16) {
					console_putc('0');
					console_putc(digits[33]);
			}
	}
	
	if (!(type&LEFT))
			while(size-->0)
					console_putc(c);
	
	while(i<precision--)
			console_putc('0');
	
	while(i-->0)
			console_putc(tmp[i]);
	
	while(size-->0)
			console_putc(' ');
}

struct color
{
	const char *name;
	enum console_color color;
};

static struct color colors[] = {
	{"{black}", console_black},
	{"{blue}", console_blue},
	{"{green}", console_green},
	{"{cyan}", console_cyan},
	{"{red}", console_red},
	{"{magenta}", console_magenta},
	{"{brown}", console_brown},
	{"{light-gray}", console_light_gray},
	{"{dark-gray}", console_dark_gray},
	{"{light-blue}", console_light_blue},
	{"{light-green}", console_light_green},
	{"{light-cyan}", console_light_cyan},
	{"{light-red}", console_light_red},
	{"{light-magenta}", console_light_magenta},
	{"{yellow}", console_yellow},
	{"{white}", console_white}
};

int vprintf(const char *format, va_list arg)
{
        int len;
        char *s;
		const char *fmt = format;

        int flags;              /* flags to number() */
		enum console_color fg = console_get_fg();


        int field_width;        /* width of output field */
        int precision;          /* min. # of digits for integers; max
                                   number of chars for from string */
        int qualifier;          /* 'h', 'l', or 'L' for integer fields */

        for (; *fmt ; ++fmt) {
                if (*fmt != '%') {
                        console_putc(*fmt);
                        continue;
                }
                        
                /* process flags */
                flags = 0;
                repeat:
                        ++fmt;          /* this also skips first '%' */
                        switch (*fmt) {
                                case '-': flags |= LEFT; goto repeat;
                                case '+': flags |= PLUS; goto repeat;
                                case ' ': flags |= SPACE; goto repeat;
                                case '#': flags |= SPECIAL; goto repeat;
                                case '0': flags |= ZEROPAD; goto repeat;
                                }
                
                /* get field width */
                field_width = -1;
                if (is_digit(*fmt))
                        field_width = skip_atoi(&fmt);
                else if (*fmt == '*') {
                        /* it's the next argument */
                        field_width = va_arg(arg, int);
                        if (field_width < 0) {
                                field_width = -field_width;
                                flags |= LEFT;
                        }
                }


                /* get the precision */
                precision = -1;
                if (*fmt == '.') {
                        ++fmt;  
                        if (is_digit(*fmt))
                                precision = skip_atoi(&fmt);
                        else if (*fmt == '*') {
                                /* it's the next argument */
                                precision = va_arg(arg, int);
                        }
                        if (precision < 0)
                                precision = 0;
                }


                /* get the conversion qualifier */
                qualifier = -1;
                if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
                        qualifier = *fmt;
                        ++fmt;
                }
				
				if(*fmt == '{')
				{
					if(fmt[1] == '}')
					{
						console_fg(fg);
						fmt += 2;
					}
					else
					{
						for(size_t i = 0; i < sizeof(colors) / sizeof(struct color); i++)
						{
							size_t len = strlen(colors[i].name);
							if(strrcmp(fmt, fmt + len, colors[i].name))
							{
								fmt += len;
								console_fg(colors[i].color);
								break;
							}
						}
					}
					--fmt;
					continue;
				}

                switch (*fmt) {
                case 'c':
                        if (!(flags & LEFT))
                                while (--field_width > 0)
                                        console_putc(' ');
                        console_putc((unsigned char) va_arg(arg, int));
                        while (--field_width > 0)
                                console_putc(' ');
                        break;


                case 's':
                        s = va_arg(arg, char *);
                        len = strlen(s);

                        if (precision < 0)
                                precision = len;
                        else if (len > precision)
                                len = precision;

                        if (!(flags & LEFT))
                                while (len < field_width--)
                                        console_putc(' ');

						console_puts(s);

                        while (len < field_width--)
                                console_putc(' ');
                        break;


                case 'o':
                        number(va_arg(arg, unsigned long), 8,
                                field_width, precision, flags);
                        break;
						
                case 'q':
                        console_fg((enum console_color)va_arg(arg, int));
                        break;
				
                case 'r':
                        console_fg(fg);
                        break;

                case 'p':
                        if (field_width == -1) {
                                field_width = 8;
                                flags |= ZEROPAD;
                        }
                        number((unsigned long)va_arg(arg, void *), 16,
                                field_width, precision, flags);
                        break;


                case 'x':
                        flags |= SMALL;
                case 'X':
                        number(va_arg(arg, unsigned long), 16,
                                field_width, precision, flags);
                        break;


                case 'd':
                case 'i':
                        flags |= SIGN;
                case 'u':
                        number(va_arg(arg, unsigned long), 10,
                                field_width, precision, flags);
                        break;
                case 'b':
                        number(va_arg(arg, unsigned long), 2,
                                field_width, precision, flags);
                        break;

                default:
                        if (*fmt != '%')
                                console_putc('%');
                        if (*fmt)
                                console_putc(*fmt);
                        else
                                --fmt;
                        break;
                }
        }
		console_fg(fg);
		return 0;
}