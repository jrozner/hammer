#include "parser_internal.h"


typedef struct {
  const HParser *p;
  int64_t lower;
  int64_t upper;
} HRange;

static HParseResult* parse_int_range(void *env, HParseState *state) {
  HRange *r_env = (HRange*)env;
  HParseResult *ret = h_do_parse(r_env->p, state);
  if (!ret || !ret->ast)
    return NULL;
  switch(ret->ast->token_type) {
  case TT_SINT:
    if (r_env->lower <= ret->ast->sint && r_env->upper >= ret->ast->sint)
      return ret;
    else
      return NULL;
  case TT_UINT:
    if ((uint64_t)r_env->lower <= ret->ast->uint && (uint64_t)r_env->upper >= ret->ast->uint)
      return ret;
    else
      return NULL;
  default:
    return NULL;
  }
}

HCFChoice* gen_int_range(HAllocator *mm__, uint64_t low, uint64_t high, uint8_t bytes) {
  if (1 == bytes) {
    HCFChoice *cs = h_new(HCFChoice, 1);
    cs->type = HCF_CHARSET;
    cs->charset = new_charset(mm__);
    for (uint64_t i=low; i<=high; ++i) {
      charset_set(cs->charset, i, 1);
    }
    cs->action = NULL;
    return cs;
  }
  else if (1 < bytes) {
    HCFChoice *root = h_new(HCFChoice, 1);
    root->type = HCF_CHOICE;
    root->seq = h_new(HCFSequence*, 4);
    root->seq[0] = h_new(HCFSequence, 1);
    root->seq[0]->items = h_new(HCFChoice*, 2);
    root->seq[0]->items[0] = gen_int_range(mm__, low, high, FIXME);
    root->seq[0]->items[1] = NULL;
    root->seq[1] = h_new(HCFSequence, 1);
    root->seq[1]->items = h_new(HCFChoice*, 2);
    root->seq[1]->items[0] = h_new(HCFChoice, 1);
    /* do something with root->seq[1]->items[0] */
    root->seq[1]->items[1] = NULL;
    root->seq[2] = h_new(HCFSequence, 1);
    root->seq[2]->items = h_new(HCFChoice*, 2);
    root->seq[2]->items[0] = gen_int_range(mm__, low, high, FIXME);
    root->seq[2]->items[1] = NULL;
    root->seq[3] = NULL;
    root->action = NULL;
    return root;
  }
  else { // idk why this would ever be <1, but whatever
    return NULL; 
  }
}

static HCFChoice* desugar_int_range(HAllocator *mm__, void *env) {
  HRange *r = (HRange*)env;
  HCFChoice *ret = h_new(HCFChoice, 1);
  ret->type = HCF_CHOICE;
  uint8_t bytes = r->p->env->length / 8;
  HCFSequence *seq = h_new(HCFSequence, 1);
  
}

static const HParserVtable int_range_vt = {
  .parse = parse_int_range,
  .isValidRegular = h_true,
  .isValidCF = h_true,
  .desugar = desugar_int_range,
};

const HParser* h_int_range(const HParser *p, const int64_t lower, const int64_t upper) {
  return h_int_range__m(&system_allocator, p, lower, upper);
}
const HParser* h_int_range__m(HAllocator* mm__, const HParser *p, const int64_t lower, const int64_t upper) {
  // p must be an integer parser, which means it's using parse_bits
  // TODO: re-add this check
  //assert_message(p->vtable == &bits_vt, "int_range requires an integer parser"); 

  // and regardless, the bounds need to fit in the parser in question
  // TODO: check this as well.

  HRange *r_env = h_new(HRange, 1);
  r_env->p = p;
  r_env->lower = lower;
  r_env->upper = upper;
  HParser *ret = h_new(HParser, 1);
  ret->vtable = &int_range_vt;
  ret->env = (void*)r_env;
  return ret;
}
