// o O щ Щ
#include <stdio.h>
#include <stdint.h>     // Расширяем числовые типы - в место char - int8_t
#include <stdlib.h>
#include <sys/stat.h>
//#include <sys/dir.h>
//#include <sys/types.h>
//#include <unistd.h>

#define BD_VERSION 1
#define SIZE_PATH 150
//  Макросы:
#define CHAR_NEW_NAME(past, new){\
            uint8_t i=0;\
            while(past[i] = new[i]){i++;}\
        }
#define PATH(path, bd, file, bin) {\
            uint8_t i=0,j=0,k=0;\
            while( path[i] = bd[i] ){i++;}\
            path[i++] = '/';\
            while( path[i] = file[j] ){i++;j++;}\
            while( path[i] = bin[k] ){i++;k++;}\
        }



// Константы
enum {FALSE=0, TRUE=1}; // bool

// Ошибки выделении памяти, Чтении/Записи
enum {OK=0, ERR_MALLOC=-1, ERR_FOPEN=-2, ERR_SEEK=-3, ERR_PUTC=-4, ERR_GETC=-5};

// Действия над файлом
enum WORK{Create, Write, Read, Change};

// Тип поля и его размеры в таблице
enum FIELD_TYPE{KOD, INT, STRING, DATA, DECIMAL, BOOL, HIPERLINK};
static const uint8_t FIELD_SIZE[] = {4, 4, 255, 3, 4, 1, 8};

// Ошибки при Сохранении/Загрузке
enum {NO_FOLDER=2, SUCH_FOLDER_IS=1, NO_DISK_SPACE=-1, };


/* Структуры:
 *     Структуры представляют из себя односвязный список
 *     Поля структур нельзя изменить
 *     Поля структур иницилизируются при чтении файла bd_info.bin
 *     Структура полей таблицы*/
struct FIELD{
    struct FIELD *field_next;    // указатель на следуюее поле
    enum FIELD_TYPE field_type;  // тип данных
    uint8_t field_size;          // размер данных в байтах
    char field_name[50];
};
//      Структура таблицы
struct TABLE{
    struct FIELD *field;
    struct TABLE *table_next;    // указатель на следуюею таблицу
    uint32_t table_number;       // номер таблицы - для гиперссылок
    uint32_t quantity_field;     // количество полей в таблице
    uint32_t quantity_kod;       // количество записей
    uint32_t kod_next;           // счетчик - выставляет КОД у новой записи
    char table_name[50];
};
//      Структура базы данных
struct BD_BIN{
    struct TABLE *table;
    uint32_t quantity_table;     // количество таблиц
    uint8_t bd_version;          // версия БД
    uint8_t allocate;            // Была ли выделена динамическая память для всей структуры?
    char bd_name[50];
};

// Прототипы:
//      Ввод/Вывод
int in_out(char*, char*, int64_t, enum WORK, void*, size_t); // Ввод/Вывод

int create(char*);                          // Создание
int write(char*, void*, size_t);            // Запись
int read(char*, void*, size_t, int64_t);    // Чтение
int change(char*, void*, size_t, int64_t);  // Изменение
int del(char*, int64_t);                    // Удаление

//      Работа со структурой БД:
int bd_new(struct BD_BIN**, char*);       // Создает и инициализирует БД

int add_table(struct BD_BIN*, char*);                   // Добавляет таблицу в БД
int add_field(struct TABLE*, char*, enum FIELD_TYPE);   // Добавляет поля таблицы

int bd_destruct(struct BD_BIN*);          // Уничтожает БД

//      Сохраняем/Загружаем структуру БД
int bd_save(struct BD_BIN*);                // Сохраняем на диск
int bd_download(struct BD_BIN**, char*);    // Загружаем с диска


/*-----БД Запись|Чтение|Изменение|Удаление------------------------------------------------------------------------------*/
// in/out *.bin
// Функция открывает файл производит запись/чтение и закрывает файл
int in_out(char *file_name, char *mode, int64_t start, enum WORK work, void *date, size_t size_date){
    int32_t simbol = 0; // запись символа при чтении

    //Открываем поток
    FILE *file=NULL;
    if((file = fopen(file_name,mode))==NULL){ return ERR_FOPEN; }

    //Смещение
    if(start){
        if(fseek(file, start, (start>0)?SEEK_SET:SEEK_END )){
            fclose(file);
            return ERR_SEEK;
        }
    }

    // Выбираем действие над данными
    switch (work) {
        case Create:
            /*Файл уже создан*/
            break;
        case Write:
            //Запись (добавление)
            for(uint8_t *c=(uint8_t *)date, *end=c+size_date; c<end; c++){
                if(putc(*c,file) == EOF){ fclose(file); return ERR_PUTC; }
            }
            break;
        case Read:
            //Чтение
            for(uint8_t *c=(uint8_t *)date, *end=c+size_date; c<end; c++){
                if((simbol = getc(file)) == EOF){ fclose(file); return ERR_GETC; }
                *c = simbol;
            }
            break;
        case Change:
            //Запись (изменение)
            for(uint8_t *c=(uint8_t *)date, *end=c+size_date; c<end; c++){
                if(putc(*c,file) == EOF){ fclose(file); return ERR_PUTC;}
            }
            break;
    }



    fclose(file);
    return OK;
}

// Создать файл
int create(char *file_name){
    return in_out(file_name,"wb",0,Create,NULL,0);
    // fopen(file_name,"wb");
}

// Запись новых данных
int write(char *file_name, void *date, size_t size_date){
    return in_out(file_name,"ab",0,Write,date,size_date);
}

// Чтение записей
int read(char *file_name, void *date, size_t size_date, int64_t start){
    return in_out(file_name,"rb",start,Read,date,size_date);
}

// Изменение данных
int change(char *file_name, void *date, size_t size_date, int64_t start){
    return in_out(file_name,"r+b",start,Change,date,size_date);
}

// Удаление - стирает запись (обнуляет её) или ставит метку что запись стерта
int del(char *file_name, int64_t start){
    /*Договоренность:
     *                   Во всех БД первый столбик таблицы: КОД (uint32_t)
     *                   КОД начинается с 1
     *                   Когда КОД == 0 - это метка удаления*/
    uint32_t kod = 0;

    return in_out(file_name,"r+b",start,Change,&kod,sizeof(kod));
}

/*-----Создание структуры БД------------------------------------------------------------------------------*/
// Создаем новую структуру БД
int bd_new(struct BD_BIN **bd, char *bd_name){
    // Выделяем память под базу данных
    bd[0] = (struct BD_BIN*)malloc(sizeof(struct BD_BIN));
    if(bd[0] == NULL){ return ERR_MALLOC; }

    // Инициализируем поля bd по умолчанию
    bd[0]->bd_version = BD_VERSION;
    bd[0]->allocate = FALSE;
    bd[0]->quantity_table = 0;
    bd[0]->table = NULL;
    CHAR_NEW_NAME(bd[0]->bd_name, bd_name)

    return OK;
}

// Добавляем таблицу в БД
int add_table(struct BD_BIN *bd, char *table_name){
    struct TABLE **table = &bd->table; // Указатель на указатель на первый элемент связного списка таблицы

    // Поиск последней таблицы в бд
    while( table[0] != NULL ){ table = &table[0]->table_next; }

    // Выделяем память под таблицу (локальный указатель)
    table[0] = (struct TABLE*)malloc(sizeof(struct TABLE));
    if(table[0] == NULL){ return ERR_MALLOC; }

    // Договоренность:
    //      Первое поле таблицы всегда КОД
    if( add_field(table[0], "Kod", KOD) ){
        // Ошибка: неполучилось выделить память для поля таблицы. Стираем таблицу
        free(table);
        table[0]=NULL;
        return ERR_MALLOC;
    }

    // Инициализируем поля table по умолчанию
    table[0]->table_number = bd->quantity_table++;
    table[0]->quantity_field = 1;
    table[0]->quantity_kod = 0;
    table[0]->kod_next = 1;
    table[0]->table_next = NULL;
    CHAR_NEW_NAME(table[0]->table_name, table_name)

    return OK;
}

// Добавляем поле в таблицу БД
int add_field(struct TABLE *table, char *field_name, enum FIELD_TYPE field_type){
    struct FIELD **field = &table->field; // Указатель на указатель на первый элемент связного списка

    // Поиск последнего поля таблицы
    while( field[0] != NULL ){ field = &field[0]->field_next; }

    // Выделяем память под поле таблицы
    field[0] = (struct FIELD*)malloc(sizeof(struct FIELD));
    if(field[0] == NULL){ return ERR_MALLOC; }

    // Инициализируем поля field по умолчанию
    field[0]->field_type = field_type;
    field[0]->field_size = FIELD_SIZE[field_type];
    field[0]->field_next = NULL;
    CHAR_NEW_NAME(field[0]->field_name,field_name)

    // Увеличиваем количество полей в таблице
    table->quantity_field++;

    return OK;
}

// Освобождаем память - уничтожаем структуру БД
int bd_destruct(struct BD_BIN *bd){
    struct TABLE *table_next=NULL, *table=bd->table;
    struct FIELD *field_next=NULL, *field=NULL;

    /* Проверяем:
     *      Если память была выделена одним блоком, то уничтожаем весь объект целиком
     *      Иначе уничтожаем каждый объект отдель*/
    if(bd->allocate){ goto bd_free; }

    // Удаляем все таблицы
    while( table != NULL ){
        // Удаляем все поля таблицы
        field=table->field;
        while( field != NULL ){
            // Очиаем память и переходим к следуюему полю
            field_next = field->field_next;
            free(field);
            field = field_next;
        }

        // Очиаем память и переходим к следуюей таблице
        table_next = table->table_next;
        free(table);
        table = table_next;
    }

    // Очиаем память БД
    bd_free:
    free(bd);
    bd=NULL;
    return OK;
}

/*-----Сохраняем/Загружаем структуру БД------------------------------------------------------------------------------*/
// Сохраняем
int bd_save(struct BD_BIN *bd){
    char path_info[SIZE_PATH],    // Максимальный размер пути к файлу
         path_table[SIZE_PATH];

    struct TABLE *table=NULL;
    struct FIELD *field=NULL;
    size_t size_table = sizeof(struct TABLE),
           size_field = sizeof(struct FIELD),
           size_struct_bd=0;    // Размер всей структуры

    // Создаем каталог для БД (1 - не ошибка просто такая БД уже есть)
    if(mkdir(bd->bd_name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)){ return SUCH_FOLDER_IS; }

    /* Создаем файл bd_info.bin:
     *    Содержит:
     *    -------------------------------------
     *    |    Шапка (size_t)     |  Данные:  |
          -------------------------------------
     *    | Размер всей структуры | Структура |
          -----------------------------------*/
     PATH(path_info, bd->bd_name,"bd_info",".bin")

     // Записываем данные в файл bd_info.bin:
     // 1) Записываем пока нулевой размер, потом изменим это значение
     if( write(path_info, &size_struct_bd, sizeof(size_struct_bd)) ){ return NO_DISK_SPACE; }

     // 2) Записываем структуру и подсчитываем её размер
     if( write(path_info, bd, sizeof(struct BD_BIN)) ){ return NO_DISK_SPACE; }
     size_struct_bd += sizeof(struct BD_BIN);

     // 3) Записываем таблицы:
     table = bd->table;
     while(table != NULL){
         size_struct_bd += size_table;
         if( write(path_info, table, size_table) ){ return NO_DISK_SPACE; }

         // 3.1) Создаем файл таблицы
         PATH(path_table,bd->bd_name,table->table_name,".bin")
         if( create(path_table) ){ return NO_DISK_SPACE; }

         // 4) Записываем поля таблицы:
         field = table->field;
         while(field != NULL){
             size_struct_bd += size_field;
             if( write(path_info,field,size_field) ){return NO_DISK_SPACE;}
             field = field->field_next;
         }
         table = table->table_next;
     }

     // 1.1) Перезаписываем размер структуры БД
     /*Тут ошибки не может быть, так как мы и так уже записывали эту строчку ранее*/
     if( change(path_info, &size_struct_bd, sizeof(size_struct_bd), 0) ){ return NO_DISK_SPACE; }

     // 5) БД нам больше не нужна
     bd_destruct(bd);

    return OK;
}

// Загружаем
int bd_download(struct BD_BIN **bd, char *bd_name){
    char path_info[SIZE_PATH];
    void *allocate = NULL,          // Динамическая память
         *offset = NULL,            // Указатель на начало свободной памяти
         *endset = NULL;            // Указатель на конец свободной памяти

    struct TABLE *table=NULL;
    struct FIELD *field=NULL;
    size_t size_table = sizeof(struct TABLE),
           size_field = sizeof(struct FIELD),
           size_struct_bd=0;    // Размер всей структуры

    PATH(path_info, bd_name, "bd_info", ".bin")

    // 1) Читаем размер структуры
    if( read(path_info, &size_struct_bd, sizeof(size_struct_bd), 0) ){ return ERR_GETC; }

    // 2) Выделяем память под структуру:
    allocate = malloc(size_struct_bd);
    if( allocate == NULL ){ return ERR_MALLOC; }

    // 3) Читаем структуру в выделенную память
    if( read(path_info, allocate, size_struct_bd, sizeof(size_struct_bd)) ){ return ERR_GETC; }
    offset = allocate;
    endset = allocate + size_struct_bd;

    // 4) Связываем прочитанные данные со структурой и между собой
    // 4.1) BD:
    bd[0] = (struct BD_BIN*)allocate;
    bd[0]->allocate = TRUE;
    offset += sizeof(struct BD_BIN);
    bd[0]->table = (offset<endset)?(struct TABLE*)offset:NULL;

    // 4.2) Table:
    for(uint32_t i=bd[0]->quantity_table; i--;){
        table = (struct TABLE*)offset;
        offset += size_table;
        table->field = (struct FIELD*)offset;

        // 4.3) Field:
        for(uint32_t j=table->quantity_field; j--;){
            field = (struct FIELD*)offset;
            offset += size_field;
            field->field_next = (j)?(struct FIELD*)offset:NULL;
        }

        table->table_next = (i)?(struct TABLE*)offset:NULL;
    }

    return OK;
}


/*-----Работаем с таблицами БД------------------------------------------------------------------------------*/












/*-----------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------------*/

















