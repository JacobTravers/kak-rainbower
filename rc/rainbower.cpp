/*
 * MIT License

 * Copyright (c) 2021 Alessandro Manca

 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct IntPair
{
    int a, b;
};

int ParseInt(const char *c, int *num_chars)
{
    int number = 0;
    if(num_chars)
    {
        *num_chars = 0;
    }

    while(*c >= '0' && *c <= '9')
    {
        number *= 10;
        number += *c - 48;
        ++c;
        if(num_chars)
        {
            ++(*num_chars);
        }
    }

    return number;
}

IntPair ParsePair(const char *c)
{
    IntPair p = {};

    int num_chars;
    p.a = ParseInt(c, &num_chars);
    c += num_chars;
    if(*c != '\0')
    {
        c++;
    }
    p.b = ParseInt(c, NULL);

    return p;
}

struct CharPosition
{
    IntPair pair;
    char c;
    int level;
};

struct CharPositionVector
{
    CharPosition *array;
    int len;
    int size;
};

void Insert(CharPositionVector *vector, CharPosition elem)
{
    if(vector->array == NULL)
    {
        vector->array = (CharPosition *)malloc(2 * sizeof(CharPosition));
        vector->size = 2;
        vector->len = 0;
    }
    else if(vector->len == vector->size)
    {
        int new_size = vector->size * 1.5f;
        int alloc_size = sizeof(CharPosition) * new_size;
        vector->array = (CharPosition *)realloc(vector->array, alloc_size);
        vector->size = new_size;
    }

    vector->array[vector->len] = elem;
    vector->len++;
}

void Free(CharPositionVector *vector)
{
    if(vector->array)
    {
        free(vector->array);
        vector->array = 0;
    }
}

struct RainbowStack
{
    CharPosition data;
    struct RainbowStack *previous;
};

void PushCharPosition(RainbowStack **s, CharPosition data)
{
    RainbowStack *new_element = (RainbowStack *)malloc(sizeof(RainbowStack));

    if(new_element)
    {
        new_element->data = data;
        new_element->previous = *s;
    }

    *s = new_element;
}

void PopCharPosition(RainbowStack **s)
{
    if(*s)
    {
        RainbowStack *previous = (*s)->previous;

        free(*s);

        *s = previous;
    }
}

void Free(RainbowStack **s)
{
    while(*s)
    {
        PopCharPosition(s);
    }
}

bool IsMaxPair(IntPair pair_a, IntPair pair_b)
{
    if(pair_a.a > pair_b.a)
    {
        return true;
    }
    else if(pair_a.a < pair_b.a)
    {
        return false;
    }
    else
    {
        if(pair_a.b >= pair_b.b)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool IsMinPair(IntPair pair_a, IntPair pair_b)
{
    if(pair_a.a < pair_b.a)
    {
        return true;
    }
    else if(pair_a.a > pair_b.a)
    {
        return false;
    }
    else
    {
        if(pair_a.b <= pair_b.b)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
}

bool IsRangeVisible(IntPair pair_a, IntPair pair_b, IntPair bound_a, IntPair bound_b)
{
    return ((IsMaxPair(pair_a, bound_a) && IsMinPair(pair_a, bound_b)) ||
            (IsMaxPair(pair_b, bound_a) && IsMinPair(pair_b, bound_b)) ||
            (IsMinPair(pair_a, bound_a) && IsMaxPair(pair_b, bound_b)));
}

char GetMatchingPair(char c)
{
    switch(c)
    {
        case ')': return '(';
        case ']': return '[';
        case '}': return '{';
        case '>': return '<';
        default: return 0;
    }
}

int InsertPair(CharPositionVector *result, RainbowStack **s, int level, char opening_bracket, CharPosition p)
{
    RainbowStack *copy = *s;
    while(copy && copy->data.c != opening_bracket)
    {
        copy = copy->previous;
    }
    if(copy)
    {
        while(*s != copy)
        {
            PopCharPosition(s);
            level--;
        }
        CharPosition p2 = (*s)->data;
        p.level = p2.level;
        Insert(result, p);
        Insert(result, p2);
        PopCharPosition(s);
        level--;
    }

    return level;
}

CharPositionVector ParseGenericFile(const char *buffer)
{
    CharPositionVector result = {};

    RainbowStack *s = NULL;

    IntPair cur_pos = { 1, 1 };

    int level = 0;

    for(const char *c = buffer; *c != '\0'; c++)
    {
        if(*c == '\n')
        {
            cur_pos.a++;
            cur_pos.b = 1;
        }
        else
        {
            CharPosition p = {};
            p.c = *c;
            p.pair = cur_pos;
            if(*c == '(' || *c == '[' || *c == '{')
            {
                p.level = level;
                PushCharPosition(&s, p);
                level++;
            }
            else if(p.c == ')' || p.c == ']' || p.c == '}')
            {
                char opening_bracket = GetMatchingPair(*c);
                level = InsertPair(&result, &s, level, opening_bracket, p);
            }
            cur_pos.b++;
        }
    }

    Free(&s);

    return result;
}

bool DeleteLessThanSign(CharPositionVector *vec)
{
    int found = -1;
    int closing = 0;

    for(int i = vec->len - 1; i >= 0; --i)
    {
        if(vec->array[i].c == '<')
        {
            if(closing)
            {
                closing--;
            }
            else
            {
                found = i;
                break;
            }
        }
        else
        {
            closing++;
        }
    }

    if(found != -1)
    {
        for(int i = found + 1; i < vec->len; ++i)
        {
            vec->array[i - 1] = vec->array[i];
        }
        vec->len--;
    }

    return (found != -1);
}

int NumPairable(CharPositionVector vec)
{
    int count = 0;

    for(int i = 0; i < vec.len; ++i)
    {
        if(vec.array[i].c == '<')
        {
            count++;
        }
        else
        {
            count--;
        }
    }

    return count;
}

struct StringParsingInfo
{
    char current_string;
    int current_string_count;
    bool closed_string;
};

void CContinueString(StringParsingInfo *info, const char *c)
{
    if((info->current_string == '\'' && *c == '\'') &&
       (*(c - 1) != '\\' || *(c - 2) == '\\'))
    {
        info->current_string = '\0';
    }
    else if((info->current_string == '\"' && *c == '\"') &&
            (*(c - 1) != '\\' || *(c - 2) == '\\'))
    {
        info->current_string = '\0';
    }
}

bool CCheckCloseMultilineComment(const char *c, const char *multiline_comment)
{
    return (*c == '/' && *(c - 1) == '*' && c != multiline_comment + 1);
}

bool CCheckStartMultilineComment(const char *c, const char *buffer, const char *last_closed_comment)
{
    return (last_closed_comment != (c - 1) && *c == '*' && c != buffer && *(c - 1) == '/');
}

bool CCheckStartLineComment(const char *c, const char *buffer, const char *last_closed_comment)
{
    return (last_closed_comment != (c - 1) && *c == '/' && c != buffer && *(c - 1) == '/');
}

CharPositionVector ParseCFile(const char *buffer, bool check_templates)
{
    CharPositionVector result = {};

    RainbowStack *s = NULL;

    IntPair cur_pos = { 1, 1 };

    int level = 0;

    StringParsingInfo info;

    info.current_string = '\0';
    info.current_string_count = 0;

    const char *multiline_comment = NULL;

    bool line_comment = false;
    const char *last_closed_comment = 0;

    CharPositionVector templates = {};

    if(check_templates)
    {
        for(const char *c = buffer; *c != '\0'; c++)
        {
            if(*c == ';' || *c == '{')
            {
                while(DeleteLessThanSign(&templates));
            }
            if(*c == '\n')
            {
                cur_pos.a++;
                cur_pos.b = 1;
                line_comment = false;
            }
            else
            {
                CharPosition p = {};
                p.c = *c;
                p.pair = cur_pos;
                if(line_comment)
                {
                }
                else if(multiline_comment)
                {
                    if(CCheckCloseMultilineComment(c, multiline_comment))
                    {
                        multiline_comment = NULL;
                        last_closed_comment = c;
                    }
                }
                else if(info.current_string == '\0' && CCheckStartMultilineComment(c, buffer, last_closed_comment))
                {
                    multiline_comment = c;
                }
                else if(info.current_string == '\0' && CCheckStartLineComment(c, buffer, last_closed_comment))
                {
                    line_comment = true;
                }
                else if(info.current_string == '\0')
                {
                    if(*c == '<')
                    {
                        Insert(&templates, p);
                    }
                    else if(*c == '>')
                    {
                        if(NumPairable(templates) > 0)
                        {
                            // NOTE ignore arrow
                            if(*(c - 1) != '-')
                            {
                                Insert(&templates, p);
                            }
                        }
                    }
                    else if(p.c == '\'' || p.c == '\"')
                    {
                        info.current_string = *c;
                    }
                }
                else if(info.current_string)
                {
                    CContinueString(&info, c);
                }
                cur_pos.b++;
            }
        }
    }

    cur_pos = { 1, 1 };

    level = 0;

    info.current_string = '\0';
    info.current_string_count = 0;

    multiline_comment = NULL;

    line_comment = false;
    last_closed_comment = 0;

    int template_i = 0;

    for(const char *c = buffer; *c != '\0'; c++)
    {
        IntPair current_template = {};
        if(template_i < templates.len)
        {
            current_template = templates.array[template_i].pair;
        }
        if(*c == '\n')
        {
            cur_pos.a++;
            cur_pos.b = 1;
            line_comment = false;
        }
        else
        {
            CharPosition p = {};
            p.c = *c;
            p.pair = cur_pos;
            if(line_comment)
            {
            }
            else if(multiline_comment)
            {
                if(CCheckCloseMultilineComment(c, multiline_comment))
                {
                    multiline_comment = NULL;
                    last_closed_comment = c;
                }
            }
            else if(info.current_string == '\0' && CCheckStartMultilineComment(c, buffer, last_closed_comment))
            {
                multiline_comment = c;
            }
            else if(info.current_string == '\0' && CCheckStartLineComment(c, buffer, last_closed_comment))
            {
                line_comment = true;
            }
            else if(info.current_string == '\0')
            {
                if(*c == '(' || *c == '[' || *c == '{' ||
                   (current_template.a == cur_pos.a && current_template.b == cur_pos.b
                    && *c == '<'))
                {
                    p.level = level;
                    PushCharPosition(&s, p);
                    level++;
                    if(current_template.a == cur_pos.a && current_template.b == cur_pos.b)
                    {
                        template_i++;
                    }
                }
                else if(p.c == ')' || p.c == ']' || p.c == '}' ||
                       (current_template.a == cur_pos.a && current_template.b == cur_pos.b
                        && p.c == '>'))
                {
                    char opening_bracket = GetMatchingPair(*c);
                    level = InsertPair(&result, &s, level, opening_bracket, p);
                    if(current_template.a == cur_pos.a && current_template.b == cur_pos.b)
                    {
                        template_i++;
                    }
                }
                else if(p.c == '\'' || p.c == '\"')
                {
                    info.current_string = *c;
                }
            }
            else if(info.current_string)
            {
                CContinueString(&info, c);
            }
            cur_pos.b++;
        }
    }

    Free(&s);
    Free(&templates);

    return result;
}

struct MultilineCommentsStack
{
    const char *ptr;
    MultilineCommentsStack *previous;
};

void PushCommentLevel(MultilineCommentsStack **s, const char *ptr)
{
    MultilineCommentsStack *new_element = (MultilineCommentsStack*)malloc(sizeof(MultilineCommentsStack));

    if(new_element)
    {
        new_element->ptr = ptr;
        new_element->previous = *s;
    }

    *s = new_element;
}

void PopCommentLevel(MultilineCommentsStack **s)
{
    if(*s)
    {
        MultilineCommentsStack *previous = (*s)->previous;

        free(*s);

        *s = previous;
    }
}

void Free(MultilineCommentsStack **s)
{
    while(*s)
    {
        PopCommentLevel(s);
    }
}

void RustContinueString(StringParsingInfo *info, const char *c)
{
    if(info->current_string == '\'' && *c == 'x' && *(c - 1) == '\\')
    {
        info->current_string = 'x';
    }
    // NOTE the first check checks for the lifetime specifier
    else if((info->current_string == '\'' && info->current_string_count == 1) ||
            ((info->current_string == '\'' && *c == '\'') &&
            (*(c - 1) != '\\' || *(c - 2) == '\\')))
    {
        info->current_string = '\0';
        info->current_string_count = 0;
        info->closed_string = true;
    }
    else if((info->current_string == '\"' && *c == '\"') &&
            (*(c - 1) != '\\' || *(c - 2) == '\\'))
    {
        info->current_string = '\0';
        info->current_string_count = 0;
        info->closed_string = true;
    }
    else if(info->current_string == 'x' && (*c == '\'' || *c < '0' || *c > '9'))
    {
        info->current_string = '\0';
        info->current_string_count = 0;
        info->closed_string = true;
    }
    else if(*(c) != '\\' || *(c - 1) == '\\')
    {
        info->current_string_count += 1;
    }
}

bool RustCheckCloseMultilineComment(const char *c, const char *multiline_comment)
{
    return (*c == '/' && *(c - 1) == '*' && c != multiline_comment + 1);
}

bool RustCheckStartMultilineComment(const char *c, const char *buffer, const char *last_closed_comment)
{
    return (last_closed_comment != (c - 1) && *c == '*' && c != buffer && *(c - 1) == '/');
}

bool RustCheckStartLineComment(const char *c, const char *buffer, const char *last_closed_comment)
{
    return (last_closed_comment != (c - 1) && *c == '/' && c != buffer && *(c - 1) == '/');
}

CharPositionVector ParseRustFile(const char *buffer, bool check_generics)
{
    CharPositionVector result = {};

    CharPositionVector generics = {};

    RainbowStack *s = NULL;

    IntPair cur_pos = { 1, 1 };

    int level = 0;

    StringParsingInfo info;
    info.current_string = '\0';
    info.current_string_count = 0;

    MultilineCommentsStack *multiline_comment = NULL;

    bool line_comment = false;
    const char *last_closed_comment = 0;

    if(check_generics)
    {
        for(const char *c = buffer; *c != '\0'; c++)
        {
            info.closed_string = false;

            if(*c == '{' || *c == '|' || *c == '^' || *c == '!')
            {
                while(DeleteLessThanSign(&generics));
            }
            if(*c == '\n')
            {
                cur_pos.a++;
                cur_pos.b = 1;
                line_comment = false;
            }
            else
            {
                CharPosition p = {};
                p.c = *c;
                p.pair = cur_pos;
                if(line_comment)
                {
                }
                else if(info.current_string == '\0' && RustCheckStartMultilineComment(c, buffer, last_closed_comment))
                {
                    PushCommentLevel(&multiline_comment, c);
                }
                else if(multiline_comment)
                {
                    if(RustCheckCloseMultilineComment(c, multiline_comment->ptr))
                    {
                        PopCommentLevel(&multiline_comment);
                        last_closed_comment = c;
                    }
                }
                else if(info.current_string == '\0' && RustCheckStartLineComment(c, buffer, last_closed_comment))
                {
                    line_comment = true;
                }
                else
                {
                    if(info.current_string)
                    {
                        RustContinueString(&info, c);
                    }
                    if(info.current_string == '\0')
                    {
                        if(*c == '<')
                        {
                            Insert(&generics, p);
                        }
                        else if(p.c == '>')
                        {
                            if(NumPairable(generics) > 0)
                            {
                                if(*(c - 1) != '-')
                                {
                                    Insert(&generics, p);
                                }
                            }
                        }
                        else if(!info.closed_string && (p.c == '\'' || p.c == '\"'))
                        {
                            info.current_string = *c;
                        }
                    }
                    cur_pos.b++;
                }
            }
        }
    }

    Free(&multiline_comment);

    cur_pos = { 1, 1 };

    level = 0;

    info.current_string = '\0';
    info.current_string_count = 0;

    line_comment = false;
    last_closed_comment = 0;

    int generic_i = 0;

    for(const char *c = buffer; *c != '\0'; c++)
    {
        info.closed_string = false;

        IntPair current_generic = {};
        if(generic_i < generics.len)
        {
            current_generic = generics.array[generic_i].pair;
        }
        if(*c == '\n')
        {
            cur_pos.a++;
            cur_pos.b = 1;
            line_comment = false;
        }
        else
        {
            CharPosition p = {};
            p.c = *c;
            p.pair = cur_pos;
            if(line_comment)
            {
            }
            else if(info.current_string == '\0' && RustCheckStartMultilineComment(c, buffer, last_closed_comment))
            {
                PushCommentLevel(&multiline_comment, c);
            }
            else if(multiline_comment)
            {
                if(RustCheckCloseMultilineComment(c, multiline_comment->ptr))
                {
                    PopCommentLevel(&multiline_comment);
                    last_closed_comment = c;
                }
            }
            else if(info.current_string == '\0' && RustCheckStartLineComment(c, buffer, last_closed_comment))
            {
                line_comment = true;
            }
            else
            {
                if(info.current_string)
                {
                    RustContinueString(&info, c);
                }
                if(info.current_string == '\0')
                {
                    if(*c == '(' || *c == '[' || *c == '{' ||
                       (current_generic.a == cur_pos.a && current_generic.b == cur_pos.b
                        && *c == '<'))
                    {
                        p.level = level;
                        PushCharPosition(&s, p);
                        level++;
                        if(current_generic.a == cur_pos.a && current_generic.b == cur_pos.b)
                        {
                            generic_i++;
                        }
                    }
                    else if(p.c == ')' || p.c == ']' || p.c == '}' ||
                            (current_generic.a == cur_pos.a && current_generic.b == cur_pos.b
                             && *c == '>'))
                    {
                        char opening_bracket = GetMatchingPair(*c);
                        level = InsertPair(&result, &s, level, opening_bracket, p);
                        if(current_generic.a == cur_pos.a && current_generic.b == cur_pos.b)
                        {
                            generic_i++;
                        }
                    }
                    else if(!info.closed_string && (p.c == '\'' || p.c == '\"'))
                    {
                        info.current_string = *c;
                    }
                }
                cur_pos.b++;
            }
        }
    }

    Free(&s);
    Free(&generics);
    Free(&multiline_comment);

    return result;
}

#define BUFFER_SIZE 500

struct String
{
    char *data;
    size_t length;
};

void CopyString(char *dest, const char *src, size_t max_size)
{
    while(max_size-- > 0)
    {
        *dest++ = *src++;
    }
    *dest = 0;
}

void AppendCStringToString(String *s, const char *to_append, size_t chars)
{
    s->data = (char *)realloc(s->data, s->length + chars + 2);
    CopyString(s->data + s->length, to_append, chars + 1);
    s->length += chars;
}

int main(int argc, const char **argv)
{
    const char *client = argv[1];
    const char *timestamp = argv[2];
    char mode = argv[3][0];

    IntPair cursor_pair = ParsePair(argv[4]);
    IntPair window_top = ParsePair(argv[5]);
    IntPair window_size = ParsePair(argv[6]);

    const char *filetype = argv[7];

    IntPair window_bottom;
    window_bottom.a = window_top.a + window_size.a;
    window_bottom.b = window_top.b + window_size.b;

    window_top.a -= 30;
    window_bottom.a += 30;

    char check_templates = argv[8][0];

    int i = 9;

    const char **colors = argv + i;
    int num_colors = 0;
    for(; i < argc && argv[i][0] != '!'; ++i)
    {
        num_colors++;
    }

    i++;

    const char **background_colors = argv + i;
    int num_background_colors = 0;
    for(; i < argc; ++i)
    {
        num_background_colors++;
    }

    String source_code = {};

    char buffer[BUFFER_SIZE] = {};
    while(int bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE - 1))
    {
        AppendCStringToString(&source_code, buffer, bytes_read);
    }

    if(!source_code.data)
    {
        return -1;
    }

    CharPositionVector result;
    if(strcmp(filetype, "c") == 0)
    {
        result = ParseCFile(source_code.data, false);
    }
    else if(strcmp(filetype, "cpp") == 0)
    {
        result = ParseCFile(source_code.data, (check_templates == 'Y'));
    }
    else if(strcmp(filetype, "rust") == 0)
    {
        result = ParseRustFile(source_code.data, (check_templates == 'Y'));
    }
    else
    {
        result = ParseGenericFile(source_code.data);
    }

    CharPosition cursor_range_a = {};
    CharPosition cursor_range_b = {};

    printf("eval -client %s set-option window rainbow %s ", client, timestamp);

    for(int k = result.len - 2; k >= 0; k -= 2)
    {
        CharPosition p = result.array[k + 1];
        CharPosition p2 = result.array[k];
        int level_index = p2.level % (num_colors);
        const char *color = colors[level_index];
        if(IsMaxPair(p.pair, window_top) && IsMinPair(p.pair, window_bottom))
        {
            printf("%d.%d,%d.%d|%s ", p.pair.a, p.pair.b, p.pair.a, p.pair.b, color);
        }
        if(IsMaxPair(p2.pair, window_top) && IsMinPair(p2.pair, window_bottom))
        {
            printf("%d.%d,%d.%d|%s ", p2.pair.a, p2.pair.b, p2.pair.a, p2.pair.b, color);
        }
        if(mode == '2')
        {
            int level_index = p2.level % (num_background_colors);
            const char *color = background_colors[level_index];
            if(IsRangeVisible(p.pair, p2.pair, window_top, window_bottom))
            {
                printf("%d.%d,%d.%d|default,%s ", p.pair.a, p.pair.b, p2.pair.a, p2.pair.b, color);
            }
        }
        else if(mode == '1')
        {
            if(((cursor_pair.a > p.pair.a) || (cursor_pair.b >= p.pair.b && cursor_pair.a == p.pair.a)) &&
               ((cursor_pair.a < p2.pair.a) || (cursor_pair.b <= p2.pair.b && cursor_pair.a == p2.pair.a)))
            {
                cursor_range_a = p;
                cursor_range_b = p2;
            }
        }
    }

    if(mode == '1')
    {
        if(cursor_range_a.c != 0)
        {
            const char *color = "rgb:003300";
            if(IsRangeVisible(cursor_range_a.pair, cursor_range_b.pair, window_top, window_bottom))
            {
                printf("%d.%d,%d.%d|default,%s ",
                       cursor_range_a.pair.a, cursor_range_a.pair.b,
                       cursor_range_b.pair.a, cursor_range_b.pair.b, color);
            }
        }
    }

    Free(&result);
    free(source_code.data);
}
