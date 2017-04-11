/*
 * Copyright (c) 2013 - 2016, Vladimir Vassilev, All Rights Reserved.
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.    
 */
/*  FILE: tecla2readline.c

   Thin wrapper implementing tecla API using readline

*/

#include <stdio.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "libtecla.h"

struct GetLine {
    int dummy;
};

char* my_line = NULL;

void process_line(char *line)
{
    if( line == NULL ) {
        fprintf(stderr, "ENOTY LINE!\n", line);
    }
    my_line = line;
    /* done here in order to prevent readline printing newline and prompt */
    rl_callback_handler_remove();
}
static GetLine my_gl;
static tcflag_t my_old_lflag;
static cc_t     my_old_vtime;
struct termios my_term;
GetLine *new_GetLine(size_t linelen, size_t histlen)
{
    /* Allow conditional parsing of the ~/.inputrc file. */
    rl_readline_name = "yangcli";
#if 1           
    /* Adjust the terminal slightly before the handler is installed. Disable
     * canonical mode processing and set the input character time flag to be
     * non-blocking.
     */
    if( tcgetattr(STDIN_FILENO, &my_term) < 0 ) {
        perror("tcgetattr");
        exit(1);
    }
    my_old_lflag = my_term.c_lflag;
    my_old_vtime = my_term.c_cc[VTIME];
    my_term.c_lflag &= ~ICANON;
    my_term.c_cc[VTIME] = 1;
    /* COMMENT LINE BELOW - see above */
    if( tcsetattr(STDIN_FILENO, TCSANOW, &my_term) < 0 ) {
        perror("tcsetattr");
        exit(1);
    }
#endif
    //rl_add_defun("change-prompt", change_prompt, CTRL('t'));

    //return (GetLine *)malloc(sizeof(struct GetLine));
    return &my_gl;
}

GetLine *del_GetLine(GetLine *gl)
{
    /* reset the old terminal setting before exiting */
    my_term.c_lflag     = my_old_lflag;
    my_term.c_cc[VTIME] = my_old_vtime;

    if( tcsetattr(STDIN_FILENO, TCSANOW, &my_term) < 0 ) {
        perror("tcsetattr");
        exit(1);
    }
    //free(gl);
    return gl;
}

static char* expand_path_with_home_prefix(const char* filename)
{
    char* expanded;
    assert(filename!=NULL);
    if(strlen(filename)>0 && filename[0]=='~') {
        struct passwd *pw = getpwuid(getuid());
        expanded = malloc(strlen(pw->pw_dir) + strlen(filename) + 1);
        sprintf(expanded,"%s%s",pw->pw_dir,&filename[1]);
    } else {
        expanded = strdup(filename);
    }
    return expanded;
}

int gl_save_history(GetLine *gl, const char *filename, const char *comment,
		    int max_lines)
{
    char* expanded_filename;
    expanded_filename = expand_path_with_home_prefix(filename);
    write_history(expanded_filename);
    free(expanded_filename);
    return 0;
}

int gl_load_history(GetLine *gl, const char *filename, const char *comment)
{
    char* expanded_filename;
    if(comment != NULL) {
        assert(strlen(comment)==1);
        history_comment_char = comment[0];
    }
    expanded_filename = expand_path_with_home_prefix(filename);
    read_history(expanded_filename);
    free(expanded_filename);
    return 0;
}

struct WordCompletion {
    int dummy;
};

static WordCompletion my_word_completion;

CplMatchFn * tecla_match_fn=NULL;

void* tecla_match_fn_data=NULL;

static char** my_completions;
static unsigned int my_completion_counter=0;
static unsigned int my_completions_max_len=0;

static int my_cpl_word_start; /* value determined by the completion callback strips the prefix base e.g. path /interfaces/.../ or optional arg prefix "--" */

static void add_cpl_prefix(const char* text, int start)
{
    char* str;
    int prefix_len;
    if(my_completions==NULL || my_completions[0]==NULL) {
        return;
    }
    prefix_len = my_cpl_word_start-start;
    str = malloc(prefix_len + strlen(my_completions[0])+1);
    memcpy(str,text,prefix_len);
    strcpy(str+prefix_len, my_completions[0]);
    free(my_completions[0]);
    my_completions[0]=str;
}
char** my_completion (const char *text, int start, int end)
{
    my_completions = malloc(1024*sizeof(char*));
    rl_attempted_completion_over = 1;
    rl_completion_query_items=64;
    tecla_match_fn(&my_word_completion, tecla_match_fn_data, rl_line_buffer, end);
    if(my_completion_counter==0) {
        free(my_completions);
        return NULL;
    } else {
        //rl_display_match_list (my_completions, my_completion_counter, my_completions_max_len);
        add_cpl_prefix(text, start);
        my_completions[my_completion_counter]=NULL;
        my_completion_counter=0;
        my_completions_max_len=0;
        return my_completions;
    }
}

int gl_customize_completion(GetLine *gl, void *data, CplMatchFn *match_fn)
{
    assert(tecla_match_fn==NULL);
    assert(tecla_match_fn_data==NULL);
    tecla_match_fn = match_fn;
    tecla_match_fn_data = data;

    rl_attempted_completion_function = my_completion;

    return 0;
}

int gl_lookup_history(GetLine *gl, unsigned long id, GlHistoryLine *line)
{
    line = NULL;
    return 0;
}

int gl_show_history(GetLine *gl, FILE *fp, const char *fmt, int all_groups,
		    int max_lines)
{
    int i;
    HIST_ENTRY *h;
    if(max_lines=-1) {
        max_lines=history_length;
    }
    for(i=0;i<max_lines;i++) {
        h = history_get(history_base+history_length-max_lines+i);
        if(h!=NULL) {
            printf("[%d] %s\n",history_base+history_length-max_lines+i,h->line);
        } else {
            break;
        }
    }
    return 0;
}

int gl_echo_mode(GetLine *gl, int enable)
{
    return 0;
}

unsigned int inactivity_sec=0;
unsigned int inactivity_nsec=0;
GlTimeoutFn* my_timeout_fn;
void* my_timeout_fn_data;

int gl_inactivity_timeout(GetLine *gl, GlTimeoutFn *timeout_fn, void *data,
		   unsigned long sec, unsigned long nsec)
{
    inactivity_sec = sec;
    inactivity_nsec = nsec;
    my_timeout_fn = timeout_fn;
    my_timeout_fn_data = data;
    return 0;
}

int gl_normal_io(GetLine *gl)
{
    return 0;
}

GlReturnStatus return_status;
char *gl_get_line(GetLine *gl, const char *prompt, const char *start_line,
		  int start_pos)
{
    struct timeval tv;
    char* line = NULL;
#if 0 /*blocking mode*/
    line=readline(prompt);
    return_status=GLR_NEWLINE;
#else
    {
        fd_set fds;
        if(my_line != NULL) {
            free(my_line);
            my_line=NULL;
        }
        tv.tv_sec=inactivity_sec;
        tv.tv_usec=inactivity_nsec/1000;

        rl_callback_handler_install(prompt, process_line);
        while(1) {
            FD_ZERO(&fds);
            FD_SET(fileno(stdin), &fds);

            if( select(FD_SETSIZE, &fds, NULL, NULL, &tv) < 0) {
                perror("Terminating");
                del_GetLine(gl);

                exit(0);
            }
   
            if( FD_ISSET(fileno(stdin), &fds) ) {
                rl_callback_read_char();                
            } else {
                GlAfterTimeout after_timeout = my_timeout_fn(gl, my_timeout_fn_data);

                if(after_timeout == GLTO_ABORT) {
                    break;
                } else if(after_timeout == GLTO_CONTINUE) {
                    tv.tv_sec=inactivity_sec;
                    tv.tv_usec=inactivity_nsec/1000;
                    continue;
                } else if (after_timeout == GLTO_REFRESH) {
                    rl_callback_handler_install(prompt, process_line);
                    continue;
                } else {
                    assert(0);
                }
            }
            if(my_line!=NULL) {
                line = my_line;
                break;
            }
        }
    }
#endif
    if(line!=NULL) {
        return_status=GLR_NEWLINE;
        add_history(line);
    } else {
        return_status=GLR_TIMEOUT;
    }
    return line;
}

GlReturnStatus gl_return_status(GetLine *gl)
{
    return return_status;
}

void gl_clear_history(GetLine *gl, int all_groups)
{
    clear_history();
}

void gl_range_of_history(GetLine *gl, GlHistoryRange *range)
{
    range->oldest = history_base;
    range->newest = history_base + history_length;
    range->nlines = history_length;
}

int cpl_add_completion(WordCompletion *cpl, const char *line,
		       int word_start, int word_end, const char *suffix,
		       const char *type_suffix, const char *cont_suffix)
{
    char* str;
    unsigned int len;
    int suffix_len;
    suffix_len = strlen(suffix);
    len = word_end-word_start + suffix_len;
    if(len>my_completions_max_len) {
        my_completions_max_len = len;
    }
    /* we need that in order to add the prefix e.g. -- or /interfaces/.../<completion_word> */
    my_cpl_word_start = word_start;

    str=(char*)malloc(len+1);
    memcpy(str+word_end-word_start,suffix,suffix_len);
    memcpy(str,line+word_start,word_end-word_start);
    str[suffix_len + word_end-word_start] = 0;

    if(my_completion_counter==0) {
        my_completions[0]=strdup(str);
        my_completion_counter++;
    } else {
        int i;
        /* find the max common completion */
        for(i=0;i<strlen(my_completions[0]);i++) {
            if(my_completions[0][i]!=str[i]) {
                my_completions[0][i]=0;
            }
        }
    }
    my_completions[my_completion_counter] = str;

    my_completion_counter++;
    assert(strlen(cont_suffix)<=1);
    rl_completion_append_character=cont_suffix[0]; /*e.g. "=","/"," "*/

    return 0;
}

void cpl_record_error(WordCompletion *cpl, const char *errmsg)
{
    fprintf(stderr,"cpl_record_error:%s\n",errmsg);
}
