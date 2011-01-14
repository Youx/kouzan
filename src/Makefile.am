bin_PROGRAMS = kouzan
noinst_LTLIBRARIES = libnbt.la

AM_CPPFLAGS=-mcmodel=large
kouzan_SOURCES =	\
	chunk.c		\
	main.c		\
	utils.c
kouzan_LDADD =		\
	libnbt.la

libnbt_la_SOURCES =	\
	nbt.c		\
	endianness.c
