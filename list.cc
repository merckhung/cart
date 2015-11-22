/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>

#include "list.h"

void _LlAddObjectToHead(CommonLinkList **Phead, CommonLinkList *Pll) {
  if (*Phead) {
    Pll->Next = *Phead;
    (*Phead)->Next = Pll;
  } else {
    *Phead = Pll;
  }
}

void _LlAddObjectToTail(CommonLinkList *Head, CommonLinkList *Pll) {
  CommonLinkList *p = _LlGetTailObject(Head);
  p->Next = Pll;
}

void _LlInsertObject(CommonLinkList *Target, CommonLinkList *Pll) {
  Pll->Next = Target->Next;
  Target->Next = Pll;
}

CommonLinkList *_LlGetObjectFromHead(CommonLinkList **Phead) {
  CommonLinkList *p;
  // Check NULL Head
  if (!(*Phead)) {
    return NULL;
  }
  // Get First Object
  p = *Phead;
  // Make Second Object become First One
  *Phead = (*Phead)->Next;
  // Return
  return p;
}

CommonLinkList *_LlGetObjectFromTail(CommonLinkList *Head) {
  CommonLinkList *p;
  if (!Head) {
    return NULL;
  }
  if (!(Head->Next)) {
    return Head;
  }
  for (; Head->Next->Next; Head = Head->Next) {}
  p = Head->Next;
  Head->Next = NULL;
  return p;
}

CommonLinkList *_LlGetTailObject(CommonLinkList *Head) {
  if (!Head) {
    return Head;
  }
  for (; Head->Next; Head = Head->Next) {}
  return Head;
}

void _LlRemoveObject(CommonLinkList **Phead, CommonLinkList *Target) {
  CommonLinkList *p, *tmp;
  if (*Phead == Target) {
    if ((*Phead)->Next) {
      p = *Phead;
      *Phead = (*Phead)->Next;
      free(p);
    } else {
      free(*Phead);
      *Phead = NULL;
    }
    return;
  }
  for (p = *Phead; p->Next; p = p->Next) {
    if (p->Next == Target) {
      tmp = p->Next;
      p->Next = p->Next->Next;
      free(tmp);
      return;
    }
  }
}

void _LlFreeAllObject(CommonLinkList *Head) {
  CommonLinkList *p, *pp;
  if (!Head) {
    return;
  }
  if (!(Head->Next)) {
    free(Head);
    return;
  }
  pp = NULL;
  for (p = Head->Next;; pp = p, p = p->Next) {
    if (pp) {
      free(pp);
    }
    if (!p) {
      break;
    }
  }
}
