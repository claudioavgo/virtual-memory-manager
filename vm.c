/*
----------------------------------------------------------------------------------
|                                                                                |
| CLASS:    OPERATING SYSTEMS                                                    |
| AUTHOR:   Cláudio Alves Gonçalves de Oliveira                                  |
| EMAIL:    hi@claudioav.com                                                     |
| DATE:     21/05/2024                                                           |
| VERSION:  1.0                                                                  |
| LICENSE:  MIT License                                                          |
|                                                                                |
| DESCRIPTION: Designing a Virtual Memory Manager, P-51, Operating System        |
| Concepts, Silberschatz.                                                        |
|                                                                                |
----------------------------------------------------------------------------------
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define PAGE_SIZE 256     // TAMANHO DE BYTES DENTRO DE UMA PÁGINA
#define PAGE_COUNT 256    // QUANTIDADE DE PÁGINAS DENTRO DA MEMÓRIA SECUNDÁRIA
#define MEMORY_SIZE 65536 // TAMANHO TOTAL DA MEMÓRIA SECUNDÁRIA

int ALGORITHM_TYPE = 0;

// BACKING_STORE.bin é a memória secundária, arquivo binário.
// Ele tem 256 páginas e cada página tem 256 bytes e cada byte representa uma instrução.
// Ou seja, cada página possui 256 instruções.

// Transformar enderço virtual em binário, selecionar os últimos 8 bits (offset) instrução dentro da página, e o número da página são os bits sobrando após selecionar os 8 últimos.

// ATENÇÃO: LER O BYTE DA INSTRUÇÃO COMO UM INTEIRO COM SINAL!

typedef struct Page
{
    int virtual_address; // Número virtual da página
    int offset;          // Número do offset relacionado a instrução.
    int number;          // Número da página
    int value;           // Conteúdo da página
    int hits;            // Quantidade de acesso à página

    int id;

    struct Page *next;
} Page;

typedef struct Frame
{
    int id;            // Id do quadro
    struct Page *page; // Página correspondente ao este frame
    int linked_page;
    int hits; // Acessos a este frame
    int last_access_time;

    struct Frame *next; // Próximo frame da fila
    struct Frame *prev; // Próximo frame da fila
} Frame;

// Estrutura da TLB (Algoritimo de subistituição de páginas para a TLB deverá ser um FIFO)
int physical_memory_counter = 0;
int tlb_counter = 0;
int reset_pyshical_memory = 0;
int timer = 0;

void clear_file(char *file_name)
{
    FILE *arquivo = fopen(file_name, "w");
    fclose(arquivo);
}

char *dec2bin(int c)
{
    char *binary_string = (char *)malloc(17);
    if (binary_string == NULL)
    {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }
    binary_string[16] = '\0';

    for (int i = 15; i >= 0; i--)
    {
        binary_string[15 - i] = (c & (1 << i)) ? '1' : '0';
    }

    return binary_string;
}

int bin2int(char *bin)
{
    int result = 0;
    while (*bin)
    {
        result = (result << 1) + (*bin++ - '0');
    }
    return result;
}

int8_t read_instruction(int page_number, int offset)
{
    // Abre o arquivo BACKING_STORE.bin para leitura binária
    FILE *file = fopen("BACKING_STORE.bin", "rb");
    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo BACKING_STORE.bin");
        exit(EXIT_FAILURE);
    }

    // Calcula o deslocamento absoluto dentro do arquivo para a página e o offset
    long absolute_offset = (long)page_number * PAGE_SIZE + offset;

    // Move o ponteiro do arquivo para o deslocamento absoluto
    if (fseek(file, absolute_offset, SEEK_SET) != 0)
    {
        perror("Erro ao posicionar o cursor no arquivo");
        exit(EXIT_FAILURE);
    }

    // Lê 1 byte do arquivo
    int8_t instruction;
    fread(&instruction, sizeof(int8_t), 1, file);

    // Fecha o arquivo
    fclose(file);

    return instruction;
}

int queue_length(Frame *head)
{
    int cont = 0;

    while (head != NULL)
    {
        cont++;
        head = head->next;
    }

    return cont;
}

int queue_length_TLB(Page *head)
{
    int cont = 0;

    while (head != NULL)
    {
        cont++;
        head = head->next;
    }

    return cont;
}

void enqueue_frame(Frame **head, Frame **tail, Page *page)
{
    Frame *new_frame = (Frame *)malloc(sizeof(Frame));

    if (new_frame != NULL)
    {
        new_frame->linked_page = page->number;
        new_frame->page = page;
        new_frame->hits = 0;
        new_frame->last_access_time = timer;
        new_frame->id = physical_memory_counter;
        new_frame->next = NULL;
        new_frame->prev = NULL;

        if (*head == NULL)
        {
            // Se a lista estiver vazia, o novo quadro será tanto a cabeça quanto a cauda
            *head = new_frame;
            *tail = new_frame;
            new_frame->prev = NULL;
        }
        else
        {
            // Adiciona o novo quadro no final da lista
            (*tail)->next = new_frame;
            new_frame->prev = *tail;
            *tail = new_frame;
        }
    }

    physical_memory_counter++;
}

void dequeue_frame(Frame **head, Frame **tail)
{
    Frame *aux;

    if (*head != NULL)
    {
        aux = *head;
        *head = (*head)->next;

        if (*head != NULL)
        {
            // Se ainda houver um próximo nó, atualize o ponteiro prev dele para NULL
            (*head)->prev = NULL;
        }
        else
        {
            // Se a lista se tornar vazia após a remoção, atualize a cauda para NULL
            *tail = NULL;
        }

        free(aux);
    }
}

void enqueue_page(Page **head, Page **tail, Page *page)
{
    if (page != NULL)
    {
        page->next = NULL;
        page->id = tlb_counter;

        if (*head == NULL)
        {
            *head = page;
            *tail = page;
        }
        else
        {
            (*tail)->next = page;
            *tail = page;
        }
    }

    // Incrementando o id para o próximo.
    tlb_counter++;
}

void dequeue_page(Page **head, Page **tail)
{
    Page *aux;

    if ((*head) != NULL)
    {

        aux = *head;

        *head = (*head)->next;

        free(aux);

        if ((*head) == NULL)
            *tail = NULL;
    }
}

void fifo_enqueue_page(Page **head, Page **tail, Page *page)
{

    // Limpar o frame mais antigo
    dequeue_page(head, tail);

    if ((*tail)->id == 15)
    {
        tlb_counter = 0;
    }

    if (page != NULL)
    {
        page->id = tlb_counter;
        page->next = NULL;

        (*tail)->next = page;
        *tail = page;
    }

    // Incrementando o id para o próximo.
    tlb_counter++;
}

void fifo_enqueue_frame(Frame **head, Frame **tail, Page *page)
{
    // Limpar o frame mais antigo
    dequeue_frame(head, tail);

    if ((*tail)->id == 127)
    {
        physical_memory_counter = 0;
    }

    Frame *new_frame = (Frame *)malloc(sizeof(Frame));

    if (new_frame != NULL)
    {
        new_frame->linked_page = page->number;
        new_frame->page = page;
        new_frame->hits = 0;
        new_frame->id = physical_memory_counter;
        new_frame->next = NULL;

        // Se a lista estiver vazia, o novo quadro será tanto a cabeça quanto a cauda
        if (*head == NULL)
        {
            *head = new_frame;
            *tail = new_frame;
            new_frame->prev = NULL;
        }
        else
        {
            (*tail)->next = new_frame;
            new_frame->prev = *tail;
            *tail = new_frame;
        }
    }

    // Incrementando o id para o próximo.
    physical_memory_counter++;
}

int check_queue_full(Frame *head)
{
    while (head != NULL)
    {
        if (head->id == 127)
        {
            return 1;
        }

        head = head->next;
    }

    return 0;
}

void lru_enqueue_frame(Frame **head, Page *page) {
    // Achando o frame menos usado
    Frame *temp = *head;
    Frame *frame_least_used = *head;

    // Iterar pela lista para encontrar o frame menos usado
    while (temp != NULL) {
        if (temp->last_access_time < frame_least_used->last_access_time) {
            frame_least_used = temp;
        }
        temp = temp->next;
    }

    // Criando um novo frame
    Frame *new_frame = (Frame *)malloc(sizeof(Frame));
    if (new_frame != NULL) {
        new_frame->linked_page = page->number;
        new_frame->page = page;
        new_frame->hits = 0; // Inicializando os hits do novo frame
        new_frame->id = frame_least_used->id;
        new_frame->next = NULL;
        new_frame->prev = NULL;

        // Inserindo o novo frame no início da lista
        if (*head == NULL) {
            *head = new_frame;
        } else {
            new_frame->next = *head;
            (*head)->prev = new_frame;
            *head = new_frame;
        }

        // Removendo o frame menos usado, se a lista estiver cheia
        if (frame_least_used != new_frame) {
            if (frame_least_used->prev != NULL) {
                frame_least_used->prev->next = frame_least_used->next;
            }
            if (frame_least_used->next != NULL) {
                frame_least_used->next->prev = frame_least_used->prev;
            }
            if (frame_least_used == *head) {
                *head = frame_least_used->next;
            }
            free(frame_least_used);
        }
    }
}

void printFila(FILE *p, Frame *head)
{
    // Frame *temp = head;
    while (head != NULL)
    {
        // if (head == temp)
        //     fprintf(p, "HEAD -> ");

        //fprintf(p, " [P: %d H: %d I: %d] ->", head->linked_page, head->hits, head->id);
        fprintf(p, " %d", head->linked_page);
        head = head->next;
    }
    // fprintf(p, "NULL\n");
    fprintf(p, "\n");
}

int find_physical_index_by_page_number(Frame *head, int page_number)
{
    while (head != NULL)
    {
        // printf(">> %d \n", head->linked_page);
        if (head->linked_page == page_number)
        {
            head->hits++;
            head->last_access_time = timer;
            return head->id;
        }

        head = head->next;
    }

    return -1;
}

int in_tlb(Page *head, int page_number)
{
    while (head != NULL)
    {
        if (head->number == page_number)
        {
            return head->id;
        }

        head = head->next;
    }

    return -1;
}

void read_virtual_addresses(char *filename)
{
    // ATENÇÃO!!! REMOVER ESSAS LINHAS NO CÓDIGO FINAL

    Frame *head = NULL;
    Frame *tail = NULL;

    Page *tlb_head = NULL;
    Page *tlb_tail = NULL;

    int number_of_translated_addresses = 0;
    int page_faults = 0;
    int tlb_hits = 0;

    // REMOVER LINHA ACIMA!!

    // Tratamento com arquivos
    FILE *p = fopen("correct.txt", "w");
    clear_file("debug.txt");
    //FILE *d = fopen("debug.txt", "a");
    FILE *file = fopen(filename, "r");

    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    // Buffer para armazenar cada linha
    char line[1000];

    // Lê cada linha do arquivo
    while (fgets(line, sizeof(line), file) != NULL)
    {

        number_of_translated_addresses++;
        timer++;

        int virtual_address = atoi(line);

        char *binary_string = dec2bin(virtual_address);
        fprintf(p, "Virtual address: %d ", virtual_address); // Exibe a linha lida
        // Dividir a string binária em duas partes de 8 bits cada
        char first_part[9] = {0}; // 8 bits + 1 (para o terminador nulo)
        char last_part[9] = {0};  // 8 bits + 1 (para o terminador nulo)

        // Copiar os primeiros 8 bits para first_part
        strncpy(first_part, binary_string, 8);
        // Copiar os últimos 8 bits para last_part
        strncpy(last_part, binary_string + 8, 8);

        // Converter as strings binárias de volta para inteiros
        int page_number = bin2int(first_part);
        int offset_number = bin2int(last_part);
        int instruction = read_instruction(page_number, offset_number);

        // Construindo uma nova página

        Page *page = (Page *)malloc(sizeof(Page));

        page->virtual_address = virtual_address;
        page->number = page_number;
        page->offset = offset_number;
        page->value = instruction;
        page->hits = 0;

        printf("%d ", page_number);

        // printf("%d\n", page->number);

        if (in_tlb(tlb_head, page_number) == -1)
        {
            int tlb_space = queue_length_TLB(tlb_head);

            if (tlb_space == 16)
            {
                fifo_enqueue_page(&tlb_head, &tlb_tail, page);
            }
            else
            {
                enqueue_page(&tlb_head, &tlb_tail, page);
            }
        }
        else
        {
            tlb_hits++;
        }

        // Verificar se a memória principal já possui este valor em seu conteúdo

        if (find_physical_index_by_page_number(head, page_number) == -1)
        {
            page_faults++;
            // Validar se há espaço na memória física disponível

            int memory_space = queue_length(head);

            if (memory_space == 128)
            {
                if (ALGORITHM_TYPE)
                {
                    lru_enqueue_frame(&head, page);
                }
                else
                {
                    fifo_enqueue_frame(&head, &tail, page);
                }
                // fprintf(d, "[REPLACEMENT] FIFO REPLACEMENT\n");
            }
            else
            {
                enqueue_frame(&head, &tail, page);
            }

            // printf("%d\n", tail->id);
        }

        int physical_index = find_physical_index_by_page_number(head, page_number);

        int tlb_number = in_tlb(tlb_head, page_number);

        // fprintf(p, "Page Number: %d ", page_number);
        // fprintf(p, "Offset: %d ", offset_number);
        fprintf(p, "physical_index: %d ", physical_index);
        fprintf(p, "TLB: %d ", tlb_number);
        fprintf(p, "Physical address: %d ", offset_number + (physical_index * 256));
        fprintf(p, "Value: %d\n", instruction);
    }

    fprintf(p, "Number of Translated Addresses = %d\n", number_of_translated_addresses);
    fprintf(p, "Page Faults = %d\n", page_faults);
    fprintf(p, "Page Fault Rate = %.3f\n", (float)page_faults / number_of_translated_addresses);
    fprintf(p, "TLB Hits = %d\n", tlb_hits);
    fprintf(p, "TLB Hit Rate = %.3f\n", (float)tlb_hits / number_of_translated_addresses);
    fclose(file);
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printf("Usage: ./vm <address_file> <algorithm_type (lru or fifo)>\n");
        return 1;
    }

    // Variável referente ao nome do arquivo que contém endereços de memória virtual
    char *filename = argv[1];

    if (strcmp("lru", argv[2]) == 0)
    {
        ALGORITHM_TYPE = 1;
    }

    // a

    // Realizando a leitura dos endereços
    read_virtual_addresses(filename);
}
