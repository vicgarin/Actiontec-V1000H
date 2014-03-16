/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     PRIVSEP = 258,
     USER = 259,
     GROUP = 260,
     CHROOT = 261,
     PATH = 262,
     PATHTYPE = 263,
     INCLUDE = 264,
     IDENTIFIER = 265,
     VENDORID = 266,
     LOGGING = 267,
     LOGLEV = 268,
     PADDING = 269,
     PAD_RANDOMIZE = 270,
     PAD_RANDOMIZELEN = 271,
     PAD_MAXLEN = 272,
     PAD_STRICT = 273,
     PAD_EXCLTAIL = 274,
     LISTEN = 275,
     X_ISAKMP = 276,
     X_ISAKMP_NATT = 277,
     X_ADMIN = 278,
     STRICT_ADDRESS = 279,
     ADMINSOCK = 280,
     DISABLED = 281,
     MODECFG = 282,
     CFG_NET4 = 283,
     CFG_MASK4 = 284,
     CFG_DNS4 = 285,
     CFG_NBNS4 = 286,
     CFG_AUTH_SOURCE = 287,
     CFG_SYSTEM = 288,
     CFG_RADIUS = 289,
     CFG_PAM = 290,
     CFG_LOCAL = 291,
     CFG_NONE = 292,
     CFG_ACCOUNTING = 293,
     CFG_CONF_SOURCE = 294,
     CFG_MOTD = 295,
     CFG_POOL_SIZE = 296,
     CFG_AUTH_THROTTLE = 297,
     CFG_PFS_GROUP = 298,
     CFG_SAVE_PASSWD = 299,
     RETRY = 300,
     RETRY_COUNTER = 301,
     RETRY_INTERVAL = 302,
     RETRY_PERSEND = 303,
     RETRY_PHASE1 = 304,
     RETRY_PHASE2 = 305,
     NATT_KA = 306,
     ALGORITHM_CLASS = 307,
     ALGORITHMTYPE = 308,
     STRENGTHTYPE = 309,
     SAINFO = 310,
     FROM = 311,
     REMOTE = 312,
     ANONYMOUS = 313,
     INHERIT = 314,
     EXCHANGE_MODE = 315,
     EXCHANGETYPE = 316,
     DOI = 317,
     DOITYPE = 318,
     SITUATION = 319,
     SITUATIONTYPE = 320,
     CERTIFICATE_TYPE = 321,
     CERTTYPE = 322,
     PEERS_CERTFILE = 323,
     CA_TYPE = 324,
     VERIFY_CERT = 325,
     SEND_CERT = 326,
     SEND_CR = 327,
     IDENTIFIERTYPE = 328,
     MY_IDENTIFIER = 329,
     PEERS_IDENTIFIER = 330,
     VERIFY_IDENTIFIER = 331,
     DNSSEC = 332,
     CERT_X509 = 333,
     CERT_PLAINRSA = 334,
     NONCE_SIZE = 335,
     DH_GROUP = 336,
     KEEPALIVE = 337,
     PASSIVE = 338,
     INITIAL_CONTACT = 339,
     NAT_TRAVERSAL = 340,
     NAT_TRAVERSAL_LEVEL = 341,
     PROPOSAL_CHECK = 342,
     PROPOSAL_CHECK_LEVEL = 343,
     GENERATE_POLICY = 344,
     SUPPORT_PROXY = 345,
     PROPOSAL = 346,
     EXEC_PATH = 347,
     EXEC_COMMAND = 348,
     EXEC_SUCCESS = 349,
     EXEC_FAILURE = 350,
     GSS_ID = 351,
     GSS_ID_ENC = 352,
     GSS_ID_ENCTYPE = 353,
     COMPLEX_BUNDLE = 354,
     DPD = 355,
     DPD_DELAY = 356,
     DPD_RETRY = 357,
     DPD_MAXFAIL = 358,
     XAUTH_LOGIN = 359,
     PREFIX = 360,
     PORT = 361,
     PORTANY = 362,
     UL_PROTO = 363,
     ANY = 364,
     IKE_FRAG = 365,
     ESP_FRAG = 366,
     MODE_CFG = 367,
     PFS_GROUP = 368,
     LIFETIME = 369,
     LIFETYPE_TIME = 370,
     LIFETYPE_BYTE = 371,
     STRENGTH = 372,
     SCRIPT = 373,
     PHASE1_UP = 374,
     PHASE1_DOWN = 375,
     NUMBER = 376,
     SWITCH = 377,
     BOOLEAN = 378,
     HEXSTRING = 379,
     QUOTEDSTRING = 380,
     ADDRSTRING = 381,
     UNITTYPE_BYTE = 382,
     UNITTYPE_KBYTES = 383,
     UNITTYPE_MBYTES = 384,
     UNITTYPE_TBYTES = 385,
     UNITTYPE_SEC = 386,
     UNITTYPE_MIN = 387,
     UNITTYPE_HOUR = 388,
     EOS = 389,
     BOC = 390,
     EOC = 391,
     COMMA = 392
   };
#endif
/* Tokens.  */
#define PRIVSEP 258
#define USER 259
#define GROUP 260
#define CHROOT 261
#define PATH 262
#define PATHTYPE 263
#define INCLUDE 264
#define IDENTIFIER 265
#define VENDORID 266
#define LOGGING 267
#define LOGLEV 268
#define PADDING 269
#define PAD_RANDOMIZE 270
#define PAD_RANDOMIZELEN 271
#define PAD_MAXLEN 272
#define PAD_STRICT 273
#define PAD_EXCLTAIL 274
#define LISTEN 275
#define X_ISAKMP 276
#define X_ISAKMP_NATT 277
#define X_ADMIN 278
#define STRICT_ADDRESS 279
#define ADMINSOCK 280
#define DISABLED 281
#define MODECFG 282
#define CFG_NET4 283
#define CFG_MASK4 284
#define CFG_DNS4 285
#define CFG_NBNS4 286
#define CFG_AUTH_SOURCE 287
#define CFG_SYSTEM 288
#define CFG_RADIUS 289
#define CFG_PAM 290
#define CFG_LOCAL 291
#define CFG_NONE 292
#define CFG_ACCOUNTING 293
#define CFG_CONF_SOURCE 294
#define CFG_MOTD 295
#define CFG_POOL_SIZE 296
#define CFG_AUTH_THROTTLE 297
#define CFG_PFS_GROUP 298
#define CFG_SAVE_PASSWD 299
#define RETRY 300
#define RETRY_COUNTER 301
#define RETRY_INTERVAL 302
#define RETRY_PERSEND 303
#define RETRY_PHASE1 304
#define RETRY_PHASE2 305
#define NATT_KA 306
#define ALGORITHM_CLASS 307
#define ALGORITHMTYPE 308
#define STRENGTHTYPE 309
#define SAINFO 310
#define FROM 311
#define REMOTE 312
#define ANONYMOUS 313
#define INHERIT 314
#define EXCHANGE_MODE 315
#define EXCHANGETYPE 316
#define DOI 317
#define DOITYPE 318
#define SITUATION 319
#define SITUATIONTYPE 320
#define CERTIFICATE_TYPE 321
#define CERTTYPE 322
#define PEERS_CERTFILE 323
#define CA_TYPE 324
#define VERIFY_CERT 325
#define SEND_CERT 326
#define SEND_CR 327
#define IDENTIFIERTYPE 328
#define MY_IDENTIFIER 329
#define PEERS_IDENTIFIER 330
#define VERIFY_IDENTIFIER 331
#define DNSSEC 332
#define CERT_X509 333
#define CERT_PLAINRSA 334
#define NONCE_SIZE 335
#define DH_GROUP 336
#define KEEPALIVE 337
#define PASSIVE 338
#define INITIAL_CONTACT 339
#define NAT_TRAVERSAL 340
#define NAT_TRAVERSAL_LEVEL 341
#define PROPOSAL_CHECK 342
#define PROPOSAL_CHECK_LEVEL 343
#define GENERATE_POLICY 344
#define SUPPORT_PROXY 345
#define PROPOSAL 346
#define EXEC_PATH 347
#define EXEC_COMMAND 348
#define EXEC_SUCCESS 349
#define EXEC_FAILURE 350
#define GSS_ID 351
#define GSS_ID_ENC 352
#define GSS_ID_ENCTYPE 353
#define COMPLEX_BUNDLE 354
#define DPD 355
#define DPD_DELAY 356
#define DPD_RETRY 357
#define DPD_MAXFAIL 358
#define XAUTH_LOGIN 359
#define PREFIX 360
#define PORT 361
#define PORTANY 362
#define UL_PROTO 363
#define ANY 364
#define IKE_FRAG 365
#define ESP_FRAG 366
#define MODE_CFG 367
#define PFS_GROUP 368
#define LIFETIME 369
#define LIFETYPE_TIME 370
#define LIFETYPE_BYTE 371
#define STRENGTH 372
#define SCRIPT 373
#define PHASE1_UP 374
#define PHASE1_DOWN 375
#define NUMBER 376
#define SWITCH 377
#define BOOLEAN 378
#define HEXSTRING 379
#define QUOTEDSTRING 380
#define ADDRSTRING 381
#define UNITTYPE_BYTE 382
#define UNITTYPE_KBYTES 383
#define UNITTYPE_MBYTES 384
#define UNITTYPE_TBYTES 385
#define UNITTYPE_SEC 386
#define UNITTYPE_MIN 387
#define UNITTYPE_HOUR 388
#define EOS 389
#define BOC 390
#define EOC 391
#define COMMA 392




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 171 "./cfparse.y"

	unsigned long num;
	vchar_t *val;
	struct remoteconf *rmconf;
	struct sockaddr *saddr;
	struct sainfoalg *alg;



/* Line 2068 of yacc.c  */
#line 334 "z.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


