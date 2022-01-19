#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "crypto.h"

#define MAX_BUFF 5000
#define MIN_PASS_LEN 0
#define MAX_PASS_LEN 100
#define AES_BITS 256
#define LF 10

int encrypt_file (char *in_fname, char *out_fname);
int decrypt_file (char *in_fname, char *out_fname);
int prompt_pass (char *pass_buff, int minlen, int maxlen);

void main(int argc, char *argv[]) {
    /* 2 commands available
     * enc (usage: jcrypt enc IN_FILE [-o OUT_FILE])
     * dec (usage: jcrypt dec IN_FILE [-o OUT_FILE])
     */

    if (argc > 1) {
        char *cmd = argv[1];
        if (strcmp(cmd, "enc") == 0) {
            // TODO check usage of jcrypt enc
            char *in_fname = argv[2];
            // TODO check -o flag and grab its value
            char *out_fname = NULL;
            if (argc == 5) {
                out_fname = argv[4];
            }
            if (encrypt_file(in_fname, out_fname) != 0) {
                // error
                puts("error");
                exit(1);
            }
        } else if (strcmp(cmd, "dec") == 0) {
            // TODO check usage of jcrypt dec
            char *in_fname = argv[2];
            // TODO check -o flag and grab its value
            char *out_fname = NULL;
            if (argc == 5) {
                out_fname = argv[4];
            }
            if (decrypt_file(in_fname, out_fname) != 0) {
                // error
                puts("error");
                exit(1);
            }
        }
    }
}

int encrypt_file (char *in_fname, char *out_fname) {
    // prompt for passphrase
    char pass[MAX_PASS_LEN];
    memset(pass, '\0', MAX_PASS_LEN);

    prompt_pass(pass, MIN_PASS_LEN, MAX_PASS_LEN - 1);

    // read file into plaintext;
    FILE *fp = fopen(in_fname, "r");
    if (fp == NULL) {
        printf("error opening file: %s\n", in_fname);
        exit(1);
    }

    char plaintext[MAX_BUFF];
    size_t bytes_read = fread(plaintext, sizeof(char), MAX_BUFF - 1, fp);
    plaintext[MAX_BUFF] = '\0';
    // TODO is file larger than MAX_BUFF?

    // encryption
    // key: hashed passphrase (256 bits = 32 bytes length) ?
    /* A 256 bit key */
    //unsigned char *key = (unsigned char *)"0123456789012345678901234567890";
    /* A 128 bit IV */
    unsigned char *iv = (unsigned char *)"0123456789012345";
    char ciphertext[MAX_BUFF];

    int encrypted_len = encrypt((unsigned char*) plaintext, strlen(plaintext), pass, iv, (unsigned char*) ciphertext);
    ciphertext[encrypted_len] = '\0';

    //printf("%s\n", ciphertext);
    fclose(fp);

    if (out_fname == NULL) {
        out_fname = strcat(in_fname, ".enc");
    }
    // TODO check if a file with the same name already exists

    fp = fopen(out_fname, "w");
    if (fp == NULL) {
        printf("error opening file: %s\n", out_fname);
        exit(1);
    }

    fputs(ciphertext, fp);
    fclose(fp);

    return 0;
}

int decrypt_file (char *in_fname, char *out_fname) {
    char ciphertext[MAX_BUFF];

    FILE *fp = fopen(in_fname, "r");
    if (fp == NULL) {
        printf("error opening file: %s\n", in_fname);
        exit(1);
    }

    fread(ciphertext, sizeof(char), MAX_BUFF - 1, fp);
    ciphertext[MAX_BUFF] = '\0';

    char plaintext[MAX_BUFF];
}

int prompt_pass (char *pass_buff, int minlen, int maxlen) {
    puts("Enter a passphrase: ");
    // TODO hide entered characters

    char temp_pass[MAX_BUFF];
    while (fgets(temp_pass, MAX_BUFF - 1, stdin) == NULL);
    temp_pass[MAX_BUFF] = '\0';
    temp_pass[strcspn(temp_pass, "\n")] = '\0';;

    int len = strlen(temp_pass);
    if (len == 0) {
        puts("Passphrase can not be empty.");
        return prompt_pass(pass_buff, minlen, maxlen);
    } else if (len > maxlen) {
        printf("Passphrase must not be longer than %d characters.\n", maxlen);
        return prompt_pass(pass_buff, minlen, maxlen);
    } else if (len < minlen) {
        printf("Passphrase must be longer than %d characters.\n", minlen);
        return prompt_pass(pass_buff, minlen, maxlen);
    }

    strcpy(pass_buff, temp_pass);
    return 0;
}




