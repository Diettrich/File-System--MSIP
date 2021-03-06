#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TOK_BUFSIZE 64
#define READLINE_BUFSIZE 1024

struct directory
{
    char name[128];
    char type[1];
    char *content;
    int contentlength;
    struct directory *fils;
    struct directory *frere;
    struct directory *previous;
};
typedef struct directory *directory;

int ls(char*, directory, directory);
int cd(char*, directory*, directory, directory);
int mkdir(char**, directory, directory, directory);
int poorls(directory);
int pwd(directory, directory);

//read line // read
char *read_line()
{
    size_t bufsize = READLINE_BUFSIZE;
    char *line = malloc(bufsize * sizeof(char));
    getline(&line, &bufsize, stdin);
    return line;
}

// execute function // eval && print
int execute(char **args, directory *current_position, directory root, directory home)
{
    char *core_commande = args[0];
    char **argument_commande = &args[1];
    //directory *tmp_position;

    if (!strcmp(core_commande, "exit"))
    {
        return 0;
    }
    else if (!strcmp(core_commande, "ls"))
    {
        if (argument_commande[0] == NULL || (argument_commande[0] != NULL && argument_commande[1] == NULL))
        {
            return ls(argument_commande[0], *current_position, root);
        }
        puts("on ne traite pas 2 argument ou plus.");
        return 1;
    }
    else if (!strcmp(core_commande, "cd"))
    {
        if (argument_commande[0] == NULL || (argument_commande[0] != NULL && argument_commande[1] == NULL))
        {
            return cd(argument_commande[0], current_position, root, home);
        }
        puts("on ne traite pas 2 argument ou plus.");
        return 1;
    }
    else if (!strcmp(core_commande, "mkdir"))
    {
        return mkdir(argument_commande, *current_position, root, home);
    }
    else if (!strcmp(core_commande, "pwd"))
    {
        return pwd(*current_position, root);
    }
    else
    {
        puts("command non existante");
        return 1;
    }
}

// parse string function
char **FL_sh_pareser(char *line, char *delimiteur)
{
    int bufsize = TOK_BUFSIZE, position = 0;
    // bufsize est le buffer responsable du stream
    // de la commande + arguments en cas de parse
    // de la ligne et la liste des passages d' un
    // chemin (URL) en cas de parse du lien commançant
    // par 64 et ajouter par 64 à fur et à mesure

    char *token;
    char **tokens = malloc(bufsize * sizeof(char *));
    // tokens est la fragmentation de de la ligne ou URL

    // exception ou token non defini
    if (!tokens)
    {
        fprintf(stderr, "FL-sh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    // parsing
    token = strtok(line, delimiteur);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;

        if (position >= bufsize)
        {
            bufsize += TOK_BUFSIZE;
            // élargisment du buffer (realloc)
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens)
            {
                fprintf(stderr, "FL-sh: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, delimiteur);
    }
    tokens[position] = NULL;
    return tokens;
}

// parse line
char **pasre_line(char *line)
{
    char *delimiteur = " \t\r\n\a";
    return FL_sh_pareser(line, delimiteur);
}

// parse URL (lien)
char **URLparser(char *line)
{
    char *delimiteur = "/";
    return FL_sh_pareser(line, delimiteur);
}

// ----------------------------------------- COMMANDES UTILITIES -------------------------------

// dirExist function(char *name, directory *position_temporere) return 1 si existe 0 sinon
// et qui change la position tmp dans la fonction appelante vers nouvel position
int dirExiste(const char *name, directory *position)
{
    directory var = *position;
    if (var->fils == NULL)
    {
        return 0;
    }

    if (strcmp(var->fils->name, name) == 0)
    {
        *position = var->fils;
        return 1;
    }
    var = var->fils;
    while (var != NULL)
    {
        if (strcmp(var->name, name) == 0)
        {
            *position = var;
            return 1;
        }
        var = var->frere;
    }
    return 0;
}

// int createDirectory(char *name, directory parent)
// cree un repertoire de nom: name sous un repertoire parenr: parent
int createDirectory(const char *name, directory *parent)
{
    directory var = malloc(sizeof(directory));
    directory present = malloc(sizeof(directory));
    directory previous = malloc(sizeof(directory));
    strcpy(var->name, name);
    strcpy(present->name, ".");
    strcpy(previous->name, "..");
    if ((*parent)->fils == NULL)
    {
        (*parent)->fils = var;
    }
    else
    {
        directory dernier_fils = (*parent)->fils;
        while (dernier_fils->frere != NULL)
        {
            dernier_fils = dernier_fils->frere;
        }
        dernier_fils->frere = var;
    }
    var->frere = NULL;
    var->fils = present;
    var->previous = *parent;
    present->fils = var->fils;
    present->frere = previous;
    present->previous = *parent;
    previous->fils = (*parent)->fils;
    previous->frere = (*parent)->frere;
    previous->previous = (*parent)->previous;

    return 1;
}

// ----------------------------------------- COMMANDES -----------------------------------------

// command ls
int ls(char *url, directory position, directory root)
{
    if (url != NULL)
    {
        directory tmp_position;
        tmp_position = url[0] == '/' ? root : position;
        if (strcmp(url, "/") == 0)
        {
            poorls(tmp_position);
            return 1;
        }
        const char **URL_NAMES_ARRAY = (const char**) URLparser(url);
        int i = 0;
        while (URL_NAMES_ARRAY[i] != NULL)
        {
            if (dirExiste(URL_NAMES_ARRAY[i], &tmp_position))
            {
                i++;
            }
            else
            {
                puts("repertoire non existant");
                return 1;
            }
        }
        return poorls(tmp_position);
    }
    else
    {
        return poorls(position);
    }
    fprintf(stderr, "FL-sh: error ls\n");
    exit(EXIT_FAILURE);
}

int poorls(directory position)
{
    directory var = position->fils;
    while (var != NULL)
    {
        puts(var->name);
        var = var->frere;
    }
    return 1;
}

// command cd : change directory
int cd(char *url, directory *position, directory root, directory home)
{
    directory tmp_position;
    if (url != NULL)
    {
        if (strcmp(url, "/") == 0)
        {
            *position = root;
            return 1;
        }
        tmp_position = url[0] == '/' ? root : *position;
        const char **URL_NAMES_ARRAY = (const char**) URLparser(url);
        int i = 0;
        while (URL_NAMES_ARRAY[i] != NULL)
        {
            if (dirExiste(URL_NAMES_ARRAY[i], &tmp_position))
            {
                //printf("%s\n", tmp_position->name);
                i++;
            }
            else
            {
                puts("repertoire non existant");
                return 1;
            }
        }
        *position = tmp_position;
        return 1;
    }
    else
    {
        *position = home;
        return 1;
    }
    fprintf(stderr, "FL-sh: error cd\n");
    exit(EXIT_FAILURE);
}

// command mkdir: make directory
int mkdir(char **args, directory position, directory root, directory home)
{
    if (args[0] == NULL)
    {
        puts("aucun repertoire saisie.");
        return 1;
    }
    int i = 0;
    while (args[i] != NULL)
    {
        const char **ARG_ARRAY = (const char**) URLparser(args[i]); // /anouar/zougrar => {anouar, zougrar}
        directory tmp_position = args[i][0] == '/' ? root: position;
        if (strcmp(ARG_ARRAY[i], "/") == 0)
        {
            puts("opération impossible.");
            return 1;
        }
        int j = 0;
        while (dirExiste(ARG_ARRAY[j], &tmp_position))
        {
            j++;
        }
        while (ARG_ARRAY[j] != NULL)
        {
            // createdir
            if (createDirectory(ARG_ARRAY[j], &tmp_position))
            {
                dirExiste(ARG_ARRAY[j], &tmp_position);
            }
            j++;
        }
        i++;
        puts("done");
    }
    return 1;
}

int pwd(directory position, directory root)
{
    if (position == root)
    {
        return 1;
    }
    pwd(position->previous, root);
    printf("/%s", position->name);
    return 1;
}

int main()
{
    // init /home/
    directory root = malloc(sizeof(directory));
    directory home = malloc(sizeof(directory));
    directory rootpresent = malloc(sizeof(directory));
    directory rootprevious = malloc(sizeof(directory));
    directory homepresent = malloc(sizeof(directory));
    directory homeprevious = malloc(sizeof(directory)); // precedent

    strcpy(root->name, "/");
    strcpy(home->name, "home");
    strcpy(rootpresent->name, ".");
    strcpy(rootprevious->name, "..");
    strcpy(homepresent->name, ".");
    strcpy(homeprevious->name, "..");

    root->fils = rootpresent;
    root->frere = NULL;
    root->previous = root;
    home->fils = homepresent;
    home->frere = NULL;
    home->previous = root;
    rootpresent->frere = rootprevious;
    rootpresent->fils = root->fils;
    rootpresent->previous = root->previous;
    rootprevious->frere = home;
    rootprevious->fils = root->fils;
    rootprevious->previous = root;
    homepresent->frere = homeprevious;
    homepresent->fils = home->fils;
    homepresent->previous = root;
    homeprevious->frere = NULL;
    homeprevious->fils = root->fils;
    homeprevious->previous = root;

    //pointeur position vers le lieu l utilisateur
    directory position = home;

    //REPL system (read eval print loop)
    char *command;
    char **args;
    int status;

    do
    {
        printf("$ ");
        command = read_line();
        args = pasre_line(command);
        status = execute(args, &position, root, home);
        free(command);
        free(args);
    } while (status == 1); // LOOP

    return 0;
}
