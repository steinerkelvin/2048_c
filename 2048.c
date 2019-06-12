#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifdef __linux
    #include<termios.h>
    // getch para linux
    int getch(void){
        struct termios oldattr, newattr;
        int ch;
        tcgetattr( STDIN_FILENO, &oldattr );
        newattr = oldattr;
        newattr.c_lflag &= ~( ICANON | ECHO );
        tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
        ch = getchar();
        tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
        return ch;
    }
    void clear_screen(){
        system("clear");
    }
    int getch_arrow(int ch){
        //if (ch == '\033') {
        if (ch == 27) {
            ch = getch();
            if (ch == 91){
                ch = getch();
                if(ch==65)       // cima
                    return 1;
                else if (ch==66) // baixo
                    return 2;
                else if (ch==67) // direita
                    return 3;
                else if (ch==68) // esquerda
                    return 4;
            }
        }
        return 0;
    }
#elif _WIN32
    void clear_screen(){
        system("cls");
    };
    int getch_arrow(int ch){
        if (ch==224){
            ch = getch();
            if(ch==72)       // cima
                return 1;
            else if (ch==75) // esquerda
                return 4;
            else if (ch==77) // direita
                return 3;
            else if (ch==80) // baixo
                return 2;
            return 0;
        }
    }
#endif


char RECORD_FILE[] = "./2048_record.sav";

char EMPTY_CELL[] = "        ";
char TABLE_WIDTH = 5+4*8;

int DEFAULT_BG_256COLOR = 249;
int DEFAULT_FG_256COLOR = 235;

//                       2    4    8   16   32   64  128  258  512 1024 2048
int TILE_COLORS[12] = {255, 252, 215, 202, 196, 160, 229, 228, 227, 226, 220, 233};

int score = 0;
int record = 0;


int table[4][4] = {0};
//int table[4][4] = {2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,2,4,8};

#ifdef __linux
    void set_bg_256color(int color){
        printf("\033[48;5;%dm", color);
    }

    void reset_bg_color(){
        set_bg_256color( DEFAULT_BG_256COLOR );
    }

    void reset_original_bg_color(){
        system("echo \"\\033[49m\"");
    }

    void set_fg_256color(int color){
        printf("\033[38;5;%dm", color);
    }

    void reset_fg_color(){
        set_fg_256color( DEFAULT_FG_256COLOR );
    }

    void reset_original_fg_color(){
        system("echo \"\\033[39m\"");
    }
#elif _WIN32
    void set_bg_256color(int color){}
    void reset_bg_color(){}
    void reset_original_bg_color(){}
    void set_fg_256color(int color){}
    void reset_fg_color(){}
    void reset_original_fg_color(){}
#endif

// determina a cor do background de uma peça
int get_tile_bg(int n){
    int i = log2(n)-1;
    if (i<=11)
        return TILE_COLORS[i];
    else
        return TILE_COLORS[11];
}

// determina a cor do texto de uma peça
int get_tile_fg(int n){
    int i = log2(n);
    if (i <= 11)
        return 235;
    else
        return 255;
}


// printa uma string n vezes
void repeatPrint(char c[], int n){
    int i;
    for (i=0; i<n; i++){
        printf("%s",c);
    }
}

// printa um espaçamento vertical de uma linha
void table_vpadding(int r){
    printf("|");
    int c;
    for (c=0; c<4; c++){
        int v = table[r][c];
        if (v) set_bg_256color(get_tile_bg(v));
        repeatPrint(" ",8);
        if (v) reset_bg_color();
        printf("|");
    }
    printf("\n");
}

// printa linha horizontal com largura da tabela
void table_hline(){
    repeatPrint("-",TABLE_WIDTH);
    printf("\n");
}

// printa a tabela
void render_table(){
    table_hline();
    
    int r,c;
    for (r=0; r<4; r++){
        // espacamento
        table_vpadding(r);
        
        printf("|");
        for (c=0; c<4; c++){
            int v = table[r][c];
            if (v) {
                set_bg_256color(get_tile_bg(v));
                set_fg_256color(get_tile_fg(v));
                printf("  %4d  ", v);
                reset_bg_color();
                reset_fg_color();
                printf("|");
            } else {
                repeatPrint(" ",8);
                printf("|");
            }
        }
        printf("\n");
        
        table_vpadding(r);
        
        table_hline();
    }
    
}

// printa informação sobre o jogo
void render_info(){
    printf( "SCORE: %-4d  RECORD: %-4d\n", score, record );
}

void render_info_2(){
    printf( "\nPress Q to quit.\n" );
}

void render_game(){
    render_info();
    render_table();
    render_info_2();
}


// conta a quantidade de espaços vazios da tabela
int count_empty_tiles(){
    int i,j,c=0;
    for (i=0;i<4;i++){
        for (j=0;j<4;j++){
            if (table[i][j]==0){
                c++;
            }
        }
    }
    return c;
}

// insere uma peça de valor 2 ou 4 em uma posição aleatória
void insert_random_tile(){
    int empt,n;
    empt = count_empty_tiles();
    if (empt){
        n = rand()%empt;
        int i,j,c=0;
        for (i=0;i<4;i++){
            for (j=0;j<4;j++){
                if (table[i][j]==0){
                    if (c==n){
                        table[i][j] = rand()%10 ? 2 : 4;
                    }
                    c++;
                }
            }
        }
    }
}


// inicializa a tabela inserindo duas peças
void init_table(){
    insert_random_tile();
    insert_random_tile();
}

void clean_table(){
    memset(table, 0, 4*sizeof(table[0]) );
}


void load_record(){
    if( access( RECORD_FILE, F_OK|R_OK ) != -1 ) {
        FILE *file;
        file = fopen(RECORD_FILE, "r");
        int status,new_record=0;
        status = fscanf(file, "%d", &new_record);
        if(status)
            record = new_record;
        fclose(file);
    } else {
        record = 0;
    }
}

void save_record(){
    if( access( RECORD_FILE, W_OK ) != -1 ) {  // F_OK
        FILE *file;
        file = fopen(RECORD_FILE, "w+");
        fprintf(file, "%d", record);
        fclose(file);
    }
}

void update_record(int n){
    record = n;
    save_record();
}

void add_score(int n){
    score += n;
    if (score > record){
        update_record(score);
    }
}


int move_table_hor (int dir){
    int new_row[4], cur_tile;
    int i,j,c,n;
    int moved=0;
    
    // para cada linha da tabela
    for (i=0;i<4;i++){
        n=0, cur_tile=0;
        memset(new_row, 0, 4*sizeof(new_row[0]));  // limpa o vetor para a nova linha
        // para cada colula
        for (j=0;j<4;j++){
            // determina o valor do índice da coluna baseado na direção do movimento
            if (dir == +1) // direita
                c = 3-j;
            else if (dir == -1) // esquerda
                c = j;
            // se não houver peça ativa e encontrar uma peça
            if ( !cur_tile && table[i][c] ){
                cur_tile = table[i][c];
            // se houver peça ativa e encontrar uma peça
            } else if ( cur_tile && table[i][c] ){
                // se a peça encontrada for igual a ativa
                if ( table[i][c] == cur_tile ) {
                    // adiciona à nova coluna uma peça com o dobro do valor das peças encontradas
                    new_row[n++] = cur_tile*2;
                    add_score( cur_tile*2 );
                    cur_tile=0;
                // se for diferente
                } else if ( table[i][c] != cur_tile ) {
                    // adiciona à nova coluna a peça ativa e seta a peça ativa para peça encontrada
                    new_row[n++] = cur_tile;
                    cur_tile = table[i][c];
                }
            }
        }
        if (cur_tile) new_row[n++] = cur_tile;
        
        // escreve a nova coluna na tabela de acordo com a dirteção do movimento
        for (j=0;j<4;j++){
            if (dir == +1)
                c = 3-j;
            else if (dir == -1)
                c = j;
            if ( !moved && table[i][c] != new_row[j])
                moved = 1;
            table[i][c] = new_row[j];
        }
    }
    return moved;
}

int move_table_vert (int dir){
    int new_col[4], cur_tile;
    int i,j,r,n;
    int moved=0;
    
    // para cada coluna da tabela
    for (i=0;i<4;i++){
        n=0, cur_tile=0;
        memset(new_col, 0, 4*sizeof(new_col[0]));  // limpa o vetor para a nova linha
        // para linha colula
        for (j=0;j<4;j++){
            // determina o valor do índice da linha baseado na direção do movimento
            if (dir == +1) // cima
                r = j;
            else if (dir == -1) // baixo
                r = 3-j;
            // se não houver peça ativa e encontrar uma peça
            if ( !cur_tile && table[r][i] ){
                cur_tile = table[r][i];
            // se houver peça ativa e encontrar uma peça
            } else if ( cur_tile && table[r][i] ){
                // se a peça encontrada for igual a ativa
                if ( table[r][i] == cur_tile ) {
                    // adiciona à nova coluna uma peça com o dobro do valor das peças encontradas
                    new_col[n++] = cur_tile*2;
                    add_score( cur_tile*2 );
                    cur_tile=0;
                // se for diferente
                } else if ( table[r][i] != cur_tile ) {
                    // adiciona à nova coluna a peça ativa e seta a peça ativa para peça encontrada
                    new_col[n++] = cur_tile;
                    cur_tile = table[r][i];
                }
            }
        }
        if (cur_tile) new_col[n++] = cur_tile;
        
        // escreve a nova coluna na tabela de acordo com a dirteção do movimento
        for (j=0;j<4;j++){
            if (dir == +1)
                r = j;
            else if (dir == -1)
                r = 3-j;
            if ( !moved && table[r][i] != new_col[j])
                moved = 1;
            table[r][i] = new_col[j];
        }
    }
    return moved;
}

void reset_game(){
    clean_table();
    init_table();
    clear_screen();
    render_game();
}


int main(){
    srand(time(NULL));
    
    load_record();
    
    clean_table();
    init_table();
    
    reset_bg_color();
    reset_fg_color();
    
    clear_screen();
    render_game();

    int ch,arrow;
    int moved=0;
    while (1){
        ch = getch();

        if ( (char)ch == 'R' || (char)ch == 'r' )
            reset_game();
        else if ( (char)ch=='Q' || (char)ch=='q' ){
            break;
        } else  {
            // se a tecla é uma seta
            arrow = getch_arrow(ch);
            if ( arrow ){
                if ( arrow == 1 ){
                    moved = move_table_vert(+1);
                } else if ( arrow == 2 ){
                    moved = move_table_vert(-1);
                } else if ( arrow == 3 ){
                    moved = move_table_hor(+1);
                } else if ( arrow == 4 ){
                    moved = move_table_hor(-1);
                }
                if (moved)
                    insert_random_tile();
                clear_screen();
                render_game();
                if (count_empty_tiles()==0){
                    clear_screen();
                    render_info();
                    render_table();
                    printf("GAME OVER\n");
                    printf("press R to start a new game\n");
                    ch = getch();
                    if ( (char)ch == 'R' || (char)ch == 'r' ){
                        reset_game();
                        continue;
                    } else
                        break;
                }
            }// else
                //printf("%d - %c\n", ch, ch);
        }
    }
    
    reset_original_bg_color();
    reset_original_fg_color();
    clear_screen();
    
    return 0;
}






