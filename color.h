/*
 * Copyright (c) 2011, David Reynolds <david@alwaysmovefast.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *
 *   3. Neither the name of the Owner nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* These are the ANSI codes	for	foreground text	colors */
#define	ANSI_BLACK		"\e[0;30m"
#define	ANSI_DRED		"\e[0;31m"
#define	ANSI_DGREEN		"\e[0;32m"
#define	ANSI_ORANGE		"\e[0;33m"
#define	ANSI_DBLUE		"\e[0;34m"
#define	ANSI_PURPLE		"\e[0;35m"
#define	ANSI_CYAN		"\e[0;36m"
#define	ANSI_GREY		"\e[0;37m"
#define	ANSI_DGREY		"\e[1;30m"
#define	ANSI_RED		"\e[1;31m"
#define	ANSI_GREEN		"\e[1;32m"
#define	ANSI_YELLOW		"\e[1;33m"
#define	ANSI_BLUE		"\e[1;34m"
#define	ANSI_PINK		"\e[1;35m"
#define	ANSI_LBLUE		"\e[1;36m"
#define	ANSI_WHITE		"\e[1;37m"
#define	ANSI_RESET		"\e[0m"

/* These are the ANSI codes	for	blinking foreground	text colors	*/
#define	BLINK_BLACK		"\e[0;5;30m"
#define	BLINK_DRED		"\e[0;5;31m"
#define	BLINK_DGREEN	"\e[0;5;32m"
#define	BLINK_ORANGE	"\e[0;5;33m"
#define	BLINK_DBLUE		"\e[0;5;34m"
#define	BLINK_PURPLE	"\e[0;5;35m"
#define	BLINK_CYAN		"\e[0;5;36m"
#define	BLINK_GREY		"\e[0;5;37m"
#define	BLINK_DGREY		"\e[1;5;30m"
#define	BLINK_RED		"\e[1;5;31m"
#define	BLINK_GREEN		"\e[1;5;32m"
#define	BLINK_YELLOW	"\e[1;5;33m"
#define	BLINK_BLUE		"\e[1;5;34m"
#define	BLINK_PINK		"\e[1;5;35m"
#define	BLINK_LBLUE		"\e[1;5;36m"
#define	BLINK_WHITE		"\e[1;5;37m"

/* These are the ANSI codes	for	background colors */
#define	BACK_BLACK		"\e[40m"
#define	BACK_DRED		"\e[41m"
#define	BACK_DGREEN		"\e[42m"
#define	BACK_ORANGE		"\e[43m"
#define	BACK_DBLUE		"\e[44m"
#define	BACK_PURPLE		"\e[45m"
#define	BACK_CYAN		"\e[46m"
#define	BACK_GREY		"\e[47m"

/* Other miscelaneous ANSI tags	that can be	used */
#define	ANSI_UNDERLINE	"\e[4m"	/* Underline text */
#define	ANSI_ITALIC		"\e[6m"	/* Italic text */
#define	ANSI_REVERSE	"\e[7m"	/* Reverse colors */
