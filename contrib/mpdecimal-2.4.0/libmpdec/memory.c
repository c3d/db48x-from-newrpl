/*
 * Copyright (c) 2008-2016 Stefan Krah. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */



/* Heavily modified to use static memory pool for newRPL */

/* Create 8 registers that can be reused */
#define MPD_MAX_INTERNAL_REGISTERS  16

/* With static memory storage for all registers to have the max. number of digits */
#define MPD_MAX_INTERNAL_DIGITS     2016
#define MPD_MAX_REGISTER_ALLOC      512  // THIS IS ((MPD_MAX_INTERNAL_DIGITS/MPD_RDIGITS)+SLACK)= 224+ SOME SLACK = 256




#include "mpdecimal.h"
#include <stdio.h>
#include <stdlib.h>
#include "typearith.h"
#include "memory.h"


/* Guaranteed minimum allocation for a coefficient. May be changed once
   at program start using mpd_setminalloc(). */
mpd_ssize_t MPD_MINALLOC = MPD_MINALLOC_MIN;


/* Statically allocated pool of registers */
static mpd_t MPD_Registers[MPD_MAX_INTERNAL_REGISTERS];
static mpd_uint_t MPD_StorageDigits[MPD_MAX_INTERNAL_REGISTERS*MPD_MAX_REGISTER_ALLOC];
mpd_uint_t MPD_RegistersUsed=0,MPD_RegistersSlaved=0;

#define mpd_getregnum(ptr) ( ((ptr)-MPD_StorageDigits)/MPD_MAX_REGISTER_ALLOC)
#define mpd_getregister( ptr ) (& (MPD_Registers[mpd_getregnum(ptr)]))

void mpd_memory_reset()
{
    MPD_RegistersUsed=0;
    MPD_RegistersSlaved=0;
}



void mpd_custom_free(void *ptr)
{

    mpd_uint_t reg=0;
    mpd_uint_t count;

    /* Fail if this pointer was not allocated by us */
    if(((mpd_uint_t *)ptr<MPD_StorageDigits) || ((mpd_uint_t *)ptr>MPD_StorageDigits+(MPD_MAX_INTERNAL_REGISTERS-1)*MPD_MAX_REGISTER_ALLOC)) return;

    reg=((mpd_uint_t)((mpd_uint_t *)ptr-MPD_StorageDigits))/MPD_MAX_REGISTER_ALLOC;

    /* Free any slaved blocks */
    count=1;
    while(MPD_RegistersSlaved&(1<<(reg+count))) {
        MPD_RegistersSlaved&=~(1<<(reg+count));
        MPD_RegistersUsed&=~(1<<(reg+count));
        ++count;
    }

    MPD_RegistersUsed&=~(1<<reg);
}

/* Use memory directly from the digit storage stack */
void *mpd_custom_malloc(size_t size)
{
mpd_uint_t reg=0;
mpd_uint_t mask=0;

size+=(sizeof(mpd_uint_t)-1);
size/=sizeof(mpd_uint_t);
size+=MPD_MAX_REGISTER_ALLOC-1;
size/=MPD_MAX_REGISTER_ALLOC;
mask=(1<<size)-1;

/* Find an empty register */
while(MPD_RegistersUsed&(mask<<reg)) ++reg;

/* Check if we have enough storage to fullfill the request */
if( (mask<<reg) & ~((1<<MPD_MAX_INTERNAL_REGISTERS)-1) )
    return NULL;


/* Mark it as Used */
MPD_RegistersUsed|=(mask<<reg);
/* Mark any secondary blocks as slaves */
MPD_RegistersSlaved|= (mask&~1)<<reg;

/* Also set the alloc word of the corresponding register */
MPD_Registers[reg].alloc=size*MPD_MAX_REGISTER_ALLOC;
MPD_Registers[reg].data=MPD_StorageDigits+MPD_MAX_REGISTER_ALLOC*reg;

return MPD_StorageDigits+MPD_MAX_REGISTER_ALLOC*reg;

}




void *mpd_custom_realloc(void *ptr, size_t current, size_t newsize)
{
size_t size=newsize;
mpd_uint_t maxsize;
mpd_uint_t reg=0;
mpd_uint_t mask=0;

if(ptr==NULL) return mpd_custom_malloc(size);
if(current>=size) return ptr;

/* If previous pointer wasn't allocated by us, then allocate new block and copy results */
if(((mpd_uint_t *)ptr<MPD_StorageDigits) || ((mpd_uint_t *)ptr>MPD_StorageDigits+(MPD_MAX_INTERNAL_REGISTERS-1)*MPD_MAX_REGISTER_ALLOC))
{
    void *newblock=mpd_custom_malloc(size);
    if(!newblock) return NULL;
    memcpy(newblock,ptr,current);
    return newblock;
}

size+=(sizeof(mpd_uint_t)-1);
size/=sizeof(mpd_uint_t);
size+=MPD_MAX_REGISTER_ALLOC-1;
size/=MPD_MAX_REGISTER_ALLOC;

/* We have enough */
if(size==1) return ptr;

mask=(1<<size)-1;
reg=((mpd_uint_t)((mpd_uint_t *)ptr-MPD_StorageDigits))/MPD_MAX_REGISTER_ALLOC;

/* Get maximum allocated size for this block */
maxsize=1;
while(MPD_RegistersSlaved&(1<<(reg+maxsize))) ++maxsize;

/* We have enough combined storage */
if(size<=maxsize) return ptr;

/* Mask with additional blocks required */
mask= (mask& ~((1<<maxsize)-1))<<reg;

/* Adjacent registers were used, allocate a new block */
if(MPD_RegistersUsed&mask) {
    void *newblock=mpd_custom_malloc(newsize);
    if(!newblock) return NULL;
    memcpy(newblock,ptr,current);
    mpd_custom_free(ptr);
    return newblock;
}

/* Also set the alloc word of the corresponding register */
MPD_Registers[reg].alloc=size*MPD_MAX_REGISTER_ALLOC;
MPD_Registers[reg].data=MPD_StorageDigits+MPD_MAX_REGISTER_ALLOC*reg;


/* Expand current block */
MPD_RegistersUsed|=mask;
MPD_RegistersSlaved|=mask;

return ptr;
}




/* Custom allocation and free functions */
void * __READ_ONLY__ (* mpd_mallocfunc)(size_t size) =  &mpd_custom_malloc;
void * __READ_ONLY__ (* mpd_reallocfunc)(void *ptr, size_t current, size_t size) = &mpd_custom_realloc;
void * __READ_ONLY__ (* mpd_callocfunc)(size_t nmemb, size_t size) = &mpd_callocfunc_em;
void __READ_ONLY__ (* mpd_free)(void *ptr) = &mpd_custom_free;


/* emulate calloc if it is not available */
void *
mpd_callocfunc_em(size_t nmemb, size_t size)
{
    void *ptr;
    size_t req;
    mpd_size_t overflow;

#if MPD_SIZE_MAX < SIZE_MAX
    /* full_coverage test only */
    if (nmemb > MPD_SIZE_MAX || size > MPD_SIZE_MAX) {
        return NULL;
    }
#endif

    req = mul_size_t_overflow((mpd_size_t)nmemb, (mpd_size_t)size,
                              &overflow);
    if (overflow) {
        return NULL;
    }

    ptr = mpd_mallocfunc(req);
    if (ptr == NULL) {
        return NULL;
    }
    /* used on uint32_t or uint64_t */
    memset(ptr, 0, req);

    return ptr;
}


/* malloc with overflow checking */
void *
mpd_alloc(mpd_size_t nmemb, mpd_size_t size)
{
    mpd_size_t req, overflow;

    req = mul_size_t_overflow(nmemb, size, &overflow);
    if (overflow) {
        return NULL;
    }

    return mpd_mallocfunc(req);
}

/* calloc with overflow checking */
void *
mpd_calloc(mpd_size_t nmemb, mpd_size_t size)
{
    mpd_size_t overflow;

    (void)mul_size_t_overflow(nmemb, size, &overflow);
    if (overflow) {
        return NULL;
    }

    return mpd_callocfunc(nmemb, size);
}

/* realloc with overflow checking */
void *
mpd_realloc(void *ptr, mpd_size_t currentnmemb, mpd_size_t nmemb, mpd_size_t size, uint8_t *err)
{
    void *newptr;
    mpd_size_t req, current, overflow;

    req = mul_size_t_overflow(nmemb, size, &overflow);
    if (overflow) {
        *err = 1;
        return ptr;
    }

    current = mul_size_t_overflow(currentnmemb, size, &overflow);
    if (overflow) {
        *err = 1;
        return ptr;
    }

    newptr = mpd_reallocfunc(ptr, current, req);
    if (newptr == NULL) {
        *err = 1;
        return ptr;
    }

    return newptr;
}

/* struct hack malloc with overflow checking */
void *
mpd_sh_alloc(mpd_size_t struct_size, mpd_size_t nmemb, mpd_size_t size)
{
    mpd_size_t req, overflow;

    req = mul_size_t_overflow(nmemb, size, &overflow);
    if (overflow) {
        return NULL;
    }

    req = add_size_t_overflow(req, struct_size, &overflow);
    if (overflow) {
        return NULL;
    }

    return mpd_mallocfunc(req);
}


/* Allocate a new decimal with a coefficient of length 'nwords'. In case
   of an error the return value is NULL. */
mpd_t *
mpd_qnew_size(mpd_ssize_t nwords)
{
    mpd_t *result;
    mpd_uint_t *data;

    nwords = (nwords < MPD_MINALLOC) ? MPD_MINALLOC : nwords;


    data = mpd_alloc(nwords, sizeof(mpd_uint_t));
    if (data == NULL) return NULL;


    result = mpd_getregister(data);

    result->flags = 0;
    result->exp = 0;
    result->digits = 0;
    result->len = 0;
    /* These two are set automatically from malloc to avoid calculating again */
    /*
    result->alloc = mpd_gettotalstorage(nwords);
    result->data=data;
    */
    return result;
}

/* Allocate a new decimal with a coefficient of length MPD_MINALLOC.
   In case of an error the return value is NULL. */
mpd_t *
mpd_qnew(void)
{
    return mpd_qnew_size(MPD_MINALLOC);
}

/* Allocate new decimal. Caller can check for NULL or MPD_Malloc_error.
   Raises on error. */
mpd_t *
mpd_new(mpd_context_t *ctx)
{
    mpd_t *result;

    result = mpd_qnew();
    if (result == NULL) {
        mpd_addstatus_raise(ctx, MPD_Malloc_error);
    }
    return result;
}

/*
 * Input: 'result' is a static mpd_t with a static coefficient.
 * Assumption: 'nwords' >= result->alloc.
 *
 * Resize the static coefficient to a larger dynamic one and copy the
 * existing data. If successful, the value of 'result' is unchanged.
 * Otherwise, set 'result' to NaN and update 'status' with MPD_Malloc_error.
 */
int
mpd_switch_to_dyn(mpd_t *result, mpd_ssize_t nwords, uint32_t *status)
{
    mpd_uint_t *p = result->data;

    assert(nwords >= result->alloc);

    result->data = mpd_alloc(nwords, sizeof *result->data);
    if (result->data == NULL) {
        result->data = p;
        mpd_set_qnan(result);
        mpd_set_positive(result);
        result->exp = result->digits = result->len = 0;
        *status |= MPD_Malloc_error;
        return 0;
    }

    memcpy(result->data, p, result->alloc * (sizeof *result->data));
    result->alloc = nwords;
    mpd_set_dynamic_data(result);
    return 1;
}

/*
 * Input: 'result' is a static mpd_t with a static coefficient.
 *
 * Convert the coefficient to a dynamic one that is initialized to zero. If
 * malloc fails, set 'result' to NaN and update 'status' with MPD_Malloc_error.
 */
int
mpd_switch_to_dyn_zero(mpd_t *result, mpd_ssize_t nwords, uint32_t *status)
{
    mpd_uint_t *p = result->data;

    result->data = mpd_calloc(nwords, sizeof *result->data);
    if (result->data == NULL) {
        result->data = p;
        mpd_set_qnan(result);
        mpd_set_positive(result);
        result->exp = result->digits = result->len = 0;
        *status |= MPD_Malloc_error;
        return 0;
    }

    result->alloc = nwords;
    mpd_set_dynamic_data(result);

    return 1;
}

/*
 * Input: 'result' is a static or a dynamic mpd_t with a dynamic coefficient.
 * Resize the coefficient to length 'nwords':
 *   Case nwords > result->alloc:
 *     If realloc is successful:
 *       'result' has a larger coefficient but the same value. Return 1.
 *     Otherwise:
 *       Set 'result' to NaN, update status with MPD_Malloc_error and return 0.
 *   Case nwords < result->alloc:
 *     If realloc is successful:
 *       'result' has a smaller coefficient. result->len is undefined. Return 1.
 *     Otherwise (unlikely):
 *       'result' is unchanged. Reuse the now oversized coefficient. Return 1.
 */
int
mpd_realloc_dyn(mpd_t *result, mpd_ssize_t nwords, uint32_t *status)
{
    uint8_t err = 0;

    result->data = mpd_realloc(result->data, result->alloc, nwords, sizeof *result->data, &err);
    if (!err) {
        result->alloc = nwords;
    }
    else if (nwords > result->alloc) {
        mpd_set_qnan(result);
        mpd_set_positive(result);
        result->exp = result->digits = result->len = 0;
        *status |= MPD_Malloc_error;
        return 0;
    }

    return 1;
}


