#include <sys/types.h>
#include <unistd.h>
#include <limits.h>

#include "run.h"
#include "util.h"

void *base = 0;

p_meta find_meta(p_meta *last, size_t size) {
  p_meta index = base;
  p_meta result = base;

  switch(fit_flag){//fit_flag is int type variable
    case FIRST_FIT:
    {
      Header *p, *prevp;
      Header *moreroce(unsigned);
      unsigned nunits;

      nunits = (size+sizeof(Header)-1)/sizeof(Header) + 1;
    if ((prevp = freep) == NULL) { /* no free list yet */
        base.s.ptr = freep = prevp = &base;
        base.s.size = 0;
    }


    /*
     * Iterate over the free list and find a block that is large enough to hold
     * size of data.
     */

    for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
        if (p->s.size >= nunits) { /* big enough */
            if (p->s.size == nunits) { /* exactly */
                prevp->s.ptr = p->s.ptr;
            }
            else {
                /* allocate tail end */
                p->s.size -= nunits;
                p += p->s.size;
                p->s.size = nunits;
            }
            freep = prevp;
            /* return Data part of block to user */
            return (void *)(p+1);
        }
        if (p == freep) { /* wrapped around free list */
            if ((p = morecore(nunits)) == NULL)
                return NULL; /* none left */
        }
      }
    }
    break;

    case BEST_FIT:
    {
      Header *p, *prevp;
  Header *moreroce(unsigned);
  unsigned nunits, min_size = INT_MAX;
  Header *minp = NULL, *minprevp = NULL;

  nunits = (size+sizeof(Header)-1)/sizeof(Header) + 1;
  if ((prevp = freep) == NULL) { /* no free list yet */
      base.s.ptr = freep = prevp = &base;
      base.s.size = 0;
  }


  /*
   * Iterate over the free list and find the smallest block that is large
   * enough to hold size of data.
   */

  for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
      if (p->s.size >= nunits) { /* big enough */
          if (p->s.size == nunits) { /* exactly */
              prevp->s.ptr = p->s.ptr;
              freep = prevp;
              return (void *)(p + 1);
          }
          else {
              if (minp == NULL || p->s.size < min_size) {
                  minp = p;
                  minprevp = prevp;
                  min_size = minp->s.size;
              }
          }
      }
    if (p == freep) { /* wrapped around free list */
        if (minp != NULL) {
            /* allocate tail end */
            minp->s.size -= nunits;
            minp += minp->s.size;
            minp->s.size = nunits;
            freep = minprevp;
            return (void *)(minp + 1);
        }
        if ((p = morecore(nunits)) == NULL) {
            return NULL; /* none left */
        }
    }
}
    }
    break;

    case WORST_FIT:
    {
      Header *p, *prevp;
          Header *moreroce(unsigned);
          unsigned nunits, max_size = 0;
          Header *maxp = NULL, *maxprevp = NULL;

          nunits = (size+sizeof(Header)-1)/sizeof(Header) + 1;
          if ((prevp = freep) == NULL) { /* no free list yet */
              base.s.ptr = freep = prevp = &base;
              base.s.size = 0;
          }


          /*
           * Iterate over the free list and find the largest block, that is large
           * enough to hold size of data.
           */

          for (p = prevp->s.ptr; ; prevp = p, p = p->s.ptr) {
              if (p->s.size >= nunits) { /* big enough */
                  if (p->s.size == nunits) { /* exactly */
                      prevp->s.ptr = p->s.ptr;
                      freep = prevp;
                      return (void *)(p + 1);
                  }
                  else {
                      if (maxp == NULL || p->s.size > max_size) {
                          maxp = p;
                          maxprevp = prevp;
                          max_size = maxp->s.size;
                      }
                  }
              }
              if (p == freep) { /* wrapped around free list */
                  if (maxp != NULL) {
                      /* allocate tail end */
                      maxp->s.size -= nunits;
                      maxp += maxp->s.size;
                      maxp->s.size = nunits;
                      freep = maxprevp;
                      return (void *)(maxp + 1);
                  }
                  if ((p = morecore(nunits)) == NULL)
                      return NULL; /* none left */
              }
          }
    }
    break;

  }
  return result;
}

void *m_malloc(size_t size) {
  if (!size) return NULL;
  size_t length = (sizeof(size_t) + sizeof(size_t)-1) & ~(struct(size_t) -1);
  static p_meta prev = NULL;
  //p_meta c = malloc_chunk_find(size, &prev);

  static p_meta pmeta = NULL;
  if(!pmeta){
    pmeta = sbrk(sizeof(struct metadata) + (sizeof(struct metadata)-1)&~(sizeof(struct metadata)-1));
    if(pmeta == (void*) -1){
      _exit(127);
    }
    pmeta->next = NULL;
    pmeta->prev = NULL;
    pmeta->size = 0;
    pmeta->free = 0;
  }
  static p_meta cc = pmeta;

  for(; cc&&(!cc->free || cc->size < s); *heap =cc, cc=cc->next);
  static p_meta c = cc;
  if (!c) {
    static p_meta newc = sbrk(length);
    if (newc == (void*) -1) {
        return NULL;
    }
    newc->next = NULL;
    newc->prev = prev;
    newc->size = length - sizeof(struct metadata);
    //newc->data = newc + 1;
    prev->next = newc;
    static c = newc;
  } else if (length + sizeof(size_t) < c->size) {
  //  malloc_split_next(c, length);
    static p_meta newcc = (p_meta)((char*) c + size);
    pmeta->prev = c;
    pmeta->next = c->next;
    pmeta->size = c->size - size;
    pmeta->free = 1;
    if (c->next) {
        c->next->prev = newcc;
    }
    c->next = newcc;
    c->size = size - sizeof(struct p_meta);
  }
  c->free = 0;
}

void m_free(void *ptr) {

  if (!ptr || ptr < c || ptr > sbrk(0)) return;
  cc = (p_meta) ptr - 1;
  if (cc->data != ptr) return;
  cc->free = 1;

  if (cc->next && c->next->free) {
    malloc_merge_next(cc);
  }
  if (cc->prev->free) {
    malloc_merge_next(cc = cc->prev);
  }
  if (!cc->next) {
    cc->prev->next = NULL;
    sbrk(- cc->size - sizeof(struct cc));
  }
}

void malloc_merge_next(p_meta c) {
    c->size = c->size + c->next->size + sizeof(struct p_meta);
    c->next = c->next->next;
    if (c->next) {
        c->next->prev = c;
    }
}

void* m_realloc(void* ptr, size_t size)
{
  void *newptr = malloc(size);
  if (newptr && ptr && ptr >= c && ptr <= sbrk(0)) {
      p_meta ccc = (p_meta) ptr - 1;
      if (ccc->data == ptr) {
          size_t length = c->size > size ? size : ccc->size;
          char *dst = newptr, *src = ptr;
          for (size_t i = 0; i < length; *dst = *src, ++src, ++dst, ++i);
          free(ptr);
      }
  }
  return newptr;
}
