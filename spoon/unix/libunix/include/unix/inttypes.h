#ifndef _INTTYPES_H
#define _INTTYPES_H		1



typedef	unsigned int		u_int;

typedef	char				int8_t;
typedef	short				int16_t;
typedef int					int32_t;
typedef long int			int64_t;
typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long int	uint64_t;

typedef long int			intptr_t;
typedef unsigned long int 	uintptr_t;

typedef	long long			intmax_t;
typedef	unsigned long long	uintmax_t;


#ifdef __GNUC__
static inline intmax_t imaxabs(intmax_t j)
{
	return __builtin_imaxabs(j);
}
#endif
															 

#endif

