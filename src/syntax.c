#include "syntax.h"

void compile_err(int err, char* pat, regex_t* rx, char* name, int line) {
  char estr[80];
  int l = regerror(err, rx, estr, 80);
  if (l > 80) return;
  fprintf(stderr,
          "Syntax: encountered an error in pattern \"%s\" (%s:%d): %s\n",
          pat, name, line, estr);
  return;
}

int syntax_lookup_color(char* key) {
  if (!strncmp(key, "number", 6)) return HL_NUM;
  else if (!strncmp(key, "string", 6)) return HL_STRING;
  else if (!strncmp(key, "string", 6)) return HL_STRING;
  else if (!strncmp(key, "comment", 7)) return HL_COMMENT;
  else if (!strncmp(key, "keyword", 7)) return HL_KEYWORD;
  else if (!strncmp(key, "type", 4)) return HL_TYPE;
  else if (!strncmp(key, "pragma", 6)) return HL_PRAGMA;
  else if (!strncmp(key, "predefined", 10)) return HL_PREDEF;
  else if (!strncmp(key, "match", 5)) return HL_MATCH;
  return HL_NORMAL;
}


char syntax_current_char(char** data) {
  return data && *data ? **data : '\0';
}



char* syntax_next_line(char** data) {
  static char xline[MAX_LINE_WIDTH];
  char* line = NULL;
  if (data && *data) {
    while (1) {
      if (**data == '\0') {
        *data = NULL;
        break;
      }
      if (!line) line = *data;
      if (**data == '\n') {
        **data = '\0';
        *data += 1;
        break;
      }
      *data += 1;
    }
  }
  if (!line) return NULL;
  // TODO : avoid this copy!
  strncpy(xline,line,MAX_LINE_WIDTH);
  return xline;
}



int syntax_read_extensions(syntax* c, char** data, char* name, int lineno,
                           char* line) {
  int regl = 1;
  regex_t* reg = malloc(sizeof(regex_t));
  int err;

  regcomp(reg, line, REG_EXTENDED);

  while (syntax_current_char(data) == ' ' || syntax_current_char(data) == '\t') {
    line = syntax_next_line(data);
    if (line != NULL) {
      line = strtriml(line);
      reg = realloc(reg, sizeof(regex_t) * ++regl);
      err = regcomp(&reg[regl-1], line, REG_EXTENDED);
      if (err) {
        compile_err(err, line, &reg[regl-1], name, lineno);

        for (int i = 0; i < regl; i++) regfree(&reg[regl]);
        free(reg);

        return 0;
      }
    }
  }
  c->matchlen = regl;
  c->filematch = reg;
  return 0;
}


int syntax_read_pattern(syntax* c, char** data, char* name, int lineno,
                        char* key, char* value) {
  pattern* pat = malloc(sizeof(pattern));
  size_t ln;
  int err;
  key = strtok(key, "|");
  char* no_sep = strtok(NULL, "|");
  pat->color = syntax_lookup_color(key);
  pat->needs_sep = !(no_sep && !strncmp(no_sep, "no_sep", 6));
  pat->multiline = 0;
  ln = strlen(value);
  memmove(value+1, value, ln);
  value[0] = '^';
  err = regcomp(&pat->pattern, value, REG_EXTENDED);
  if (err) {
    compile_err(err, value, &pat->pattern, name, lineno);
    regfree(&pat->pattern);
    free(pat);
    return 1;
  }

  if (syntax_current_char(data) == ' ' || syntax_current_char(data) == '\t') {
    char* line = syntax_next_line(data);
    if (line != NULL) {
      char* l = strtriml(line);
      ln = strlen(l);
      memmove(l+1, l, ln);
      l[0] = '^';
      err = regcomp(&pat->closing, l, REG_EXTENDED);
      if (err) {
        compile_err(err, l, &pat->closing, name, lineno);
        regfree(&pat->pattern);
        regfree(&pat->closing);
        free(pat);
        return 1;
      }
      pat->multiline = 1;
    }
  }

  c->npatterns++;
  c->patterns = realloc(c->patterns, sizeof(pattern)*c->npatterns);
  memmove(&c->patterns[c->npatterns-1], pat, sizeof(pattern));
  free(pat);
  return 0;
}


syntax** syntax_init(syntax** ret, int *retl, char* name, char* data) {

  if (ret == 0 || *retl == 0){
    *retl = 1;
    ret = malloc(sizeof(syntax*));
  }
  syntax* c;

  while (data != NULL) {

    c = NULL;

    int lineno = 0;

    c = malloc(sizeof(syntax));
    c->patterns = NULL;
    c->ftype = NULL;
    c->filematch = NULL;
    c->npatterns = 0;

    while (1) {
      lineno++;
      char* line = syntax_next_line(&data);
      if (line == NULL) break;
      if (*line == '\0') continue;

      char* key = strtok(line, ":");
      if (key && !strncmp(key, "split", 5)) {
        break;
      }
      char* value = strtok(NULL, ":");
      if (key && !strncmp(key, "displayname", 11)) {
        c->ftype = strdup(strtriml(value));
      } else if (key && !strncmp(key, "extensions", 10)) {
        if (syntax_read_extensions(c, &data, name, lineno, strtriml(value))) {
          syntax_free(c);
          c = NULL;
          break;
        }
      } else {
        if (value) {
          if (syntax_read_pattern(c, &data, name, lineno, key, strtriml(value))) {
            syntax_free(c);
            c = NULL;
            break;
          }
        }
      }
    }

    if (!c) {
      ret = realloc(ret, sizeof(syntax*)**retl);
      ret[*retl-1] = NULL;
      syntaxes_free(ret);
      return NULL;
    }
    ret = realloc(ret, sizeof(syntax*)**retl);
    ret[*retl-1] = c;
    *retl += 1;
  }

  ret = realloc(ret, sizeof(syntax*)**retl);
  ret[*retl-1] = NULL;
  return ret;
}

void syntax_free(syntax* s) {
  int i;
  if (s->ftype) free(s->ftype);

  if (s->filematch) {
    for (i = 0; i < s->matchlen; i++) regfree(&s->filematch[i]);
    free(s->filematch);
  }

  for (i = 0; i < s->npatterns; i++) {
    regfree(&s->patterns[i].pattern);
    if (s->patterns[i].multiline) regfree(&s->patterns[i].closing);
  }
  free(s->patterns);

  free(s);
}

void syntaxes_free(syntax** stx) {
  int i = 0;
  while (stx[i]) syntax_free(stx[i++]);
  free(stx);
}
