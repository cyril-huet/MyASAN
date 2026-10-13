# MyASAN

**MyASAN** est un petit outil de detection d'erreurs memoire ecrit en **C11**.

C'est un projet pedagogique : le but est de comprendre les pointeurs, `malloc`,
`calloc`, `realloc`, `free` et les listes chainees en gardant un code simple.

Il utilise des fonctions C classiques, des `if`, des `for`, des `while` et des
structures. Pas de bit shifts, pas de framework de test, pas de dependances
externes et pas d'instrumentation du compilateur.

> **Important :** ce projet n'est **pas** le vrai AddressSanitizer. Il detecte
> certaines erreurs mais ne surveille pas tous les acces memoire.

## Fonctionnalites

- Suit `malloc`, `calloc`, `realloc` et `free` via les macros `MY_*`.
- Detecte un `double free` et certains pointeurs invalides.
- Signale les allocations non liberees (fuites) sur demande.
- Ajoute deux zones de garde pour detecter certains debordements de tableaux.
- Indique le fichier et la ligne de l'allocation et de l'erreur.
- Indique aussi l'emplacement du premier `free` en cas de double liberation.
- Affiche les blocs actifs et des statistiques (allocations, octets, pic memoire).
- Peut enregistrer les erreurs dans un fichier texte `.log`.
- Propose un mode normal, silencieux pour la console et detaille.
- Contient **100 tests**, executes avec un petit runner maison.
- Utilise GitHub Actions pour tester GCC, Clang, macOS et les sanitizers.

## Prerequis

Un compilateur C11 (`cc`, `gcc` ou `clang`), `make` et `ar`.

Fonctionne comme une bibliotheque C sur Linux et macOS. Aucun `LD_PRELOAD`
n'est necessaire. Les tests de la CI macOS dependent de GitHub Actions.

## Compiler et tester

```sh
make            # Produit libmyasan.a
make test       # Compile et execute les 100 tests
make check      # Alias de make test
make re         # Nettoie puis recompile la bibliotheque
make clean      # Supprime les fichiers generes
```

Pour compiler avec Clang :

```sh
make clean
make CC=clang test
```

Pour verifier **le code de MyASAN** avec les vrais sanitizers :

```sh
make CC=clang sanitize
```

Le dernier mode demande un compilateur prenant en charge
`-fsanitize=address,undefined`. Ce controle ne transforme pas MyASAN en un
AddressSanitizer complet.

## Utilisation : exemple simple

Cree un fichier `demo.c` a cote du Makefile :

```c
#include <stdio.h>
#include "myasan.h"

int main(void)
{
    char *name = MY_MALLOC(8);
    if (name == NULL)
    {
        return 1;
    }

    name[0] = 'H';
    name[1] = 'i';
    name[2] = '\0';
    printf("%s\n", name);

    MY_FREE(name);
    myasan_print_summary();
    myasan_reset();
    return 0;
}
```

Puis compile-le :

```sh
make
cc -std=c11 -Iinclude demo.c libmyasan.a -o demo
./demo
```

`MY_MALLOC` et `MY_FREE` sont de simples macros qui transmettent `__FILE__`
et `__LINE__` pour retrouver l'emplacement des appels. La bibliotheque
utilise le vrai `malloc` en interne.

### Detecter un double free

```c
char *p = MY_MALLOC(20);
MY_FREE(p);
MY_FREE(p); /* MyASAN affiche une erreur. */
```

Exemple de rapport :

```text
[MyASAN] ERROR: double free at demo.c:14
  Pointer: 0x..., size: 20
  Allocated at: demo.c:12
  First freed at: demo.c:13
```

Les adresses et les numeros de ligne changent selon le programme.

### Detecter un debordement

```c
unsigned char *p = MY_MALLOC(4);
p[4] = 10;        /* Modifie la garde situee apres le bloc. */
myasan_check();   /* Detecte le probleme ici, pas au moment de l'ecriture. */
MY_FREE(p);
myasan_reset();
```

**Attention :** cet exemple ecrit volontairement hors du tableau logique.
Ne l'utilise pas dans un vrai programme applicatif.

### Detecter une fuite

```c
char *p = MY_MALLOC(64);
myasan_finish();   /* Affiche une fuite de 64 octets. */
myasan_reset();    /* Libere les blocs suivis restants. */
```

`myasan_finish()` ne libere pas les blocs et peut etre appelee plusieurs fois.
`myasan_reset()` libere les allocations restantes et remet les statistiques
a zero. Les pointeurs devenus invalides ne doivent pas etre reutilises.

## Enregistrer les rapports dans un fichier

```c
#include "myasan.h"

int main(void)
{
    if (myasan_set_log_file("myasan.log") != 0)
    {
        return 1;
    }

    myasan_set_reporting(0); /* Pas de messages sur stderr. */
    myasan_set_verbose(1);   /* Ajoute les lignes ALLOC et FREE au fichier. */

    char *p = MY_MALLOC(10);
    if (p != NULL)
    {
        MY_FREE(p);
        MY_FREE(p);
    }

    myasan_close_log();
    myasan_reset();
    return 0;
}
```

- `myasan_set_reporting(0)` coupe **uniquement les messages sur la console**.
  Le fichier de log continue de recevoir les messages.
- `myasan_set_verbose(1)` active les messages `ALLOC` et `FREE`, en plus des
  erreurs. `myasan_set_verbose(0)` revient au mode normal.
- `myasan_set_log_file("fichier.log")` ouvre ou remplace le fichier en
  **ecrasant son ancien contenu** ; renvoie `0` en cas de succes et `-1`
  en cas d'echec. En cas d'echec, l'ancien fichier reste actif.
- `myasan_close_log()` termine la journalisation. On peut l'appeler plusieurs
  fois sans probleme.
- La configuration du log et des modes persiste apres `myasan_reset()`.

## API publique

Tout est declare dans `include/myasan.h`.

| Fonction | Role |
| --- | --- |
| `MY_MALLOC(size)` | Alloue un bloc et memorise son origine. |
| `MY_CALLOC(n, size)` | Alloue un bloc initialise a zero. |
| `MY_REALLOC(ptr, size)` | Redimensionne un bloc suivi. |
| `MY_FREE(ptr)` | Verifie les gardes et libere un bloc suivi. |
| `myasan_check()` | Verifie les gardes de tous les blocs actifs. |
| `myasan_check_pointer(ptr)` | Verifie un bloc actif precis : `0`, `1`, `2`, ou `-1` si pointeur inconnu/libere. |
| `myasan_finish()` | Affiche les fuites et renvoie le nombre de blocs restants. |
| `myasan_reset()` | Libere les blocs restants et remet les compteurs a zero. |
| `myasan_get_stats()` | Renvoie les compteurs (`struct myasan_stats`). |
| `myasan_last_error()` | Renvoie le dernier type d'erreur (`enum myasan_error`). |
| `myasan_is_tracked(ptr)` | Teste si un bloc est actif. |
| `myasan_size(ptr)` | Renvoie sa taille demandee, ou `0` si inconnu/libere. |
| `myasan_count_leaks()` | Compte les blocs actifs sans les afficher. |
| `myasan_leaked_bytes()` | Totalise leurs tailles demandees. |
| `myasan_dump_blocks()` | Affiche les blocs actifs. |
| `myasan_print_summary()` | Affiche les statistiques. |
| `myasan_set_reporting(0/1)` | Coupe ou active la sortie diagnostic console. |
| `myasan_set_verbose(0/1)` | Coupe ou active les traces d'allocations/liberations. |
| `myasan_set_log_file(path)` | Ecrit les diagnostics dans un fichier, retour `0` ou `-1`. |
| `myasan_close_log()` | Ferme le fichier de log actif. |

`myasan_check()` et `myasan_check_pointer()` incrementent les compteurs
lorsqu'ils detectent une garde corrompue. Ils peuvent donc compter la meme
corruption a plusieurs appels. `myasan_last_error()` correspond a la derniere
operation verifiee ; `MY_FREE(NULL)` est accepte sans erreur.

Le `realloc` de MyASAN libere le pointeur si la nouvelle taille est zero
et renvoie `NULL`. Un `MY_MALLOC(0)` produit un bloc suivable (si l'allocation
reussit). Si un `MY_REALLOC` echoue, le bloc initial reste alloue.

### Option : remplacer les noms classiques

Apres les `#include` standards **dans tes propres fichiers C** :

```c
#define MYASAN_REPLACE
#include "myasan.h"

/* Dans ce fichier, malloc, calloc, realloc et free deviennent des macros. */
```

Cela ne modifie pas les allocations des bibliotheques tierces et n'intercepte
pas les programmes deja compiles. Ne pas activer ce mode en compilant les
sources internes de MyASAN.

## Comment fonctionne la memoire ?

Pour un `MY_MALLOC(4)` :

```text
+----------------+----------------+----------------+
| garde avant    | 4 octets utiles| garde apres    |
+----------------+----------------+----------------+
                 ^
           pointeur rendu
```

Les deux gardes contiennent un octet fixe. Lors d'un `MY_FREE`, de
`myasan_check()` ou de `myasan_check_pointer()`, MyASAN compare leur contenu.
Si une garde a change, il affiche une erreur.

Chaque allocation possede aussi une petite fiche (`struct block`) conservee
dans une **liste chainee** : adresse, taille, fichier, ligne et etat
libere/non libere. Les fiches des blocs liberes restent jusqu'au `reset` pour
detecter un second `free`.

## Organisation des fichiers

```text
MyASAN/
├── .github/workflows/ci.yml   # Tests automatiques GitHub
├── include/myasan.h           # Fonctions publiques et macros
├── src/
│   ├── internal.h             # Structure block et fonctions internes
│   ├── myasan.c               # Allocations, liberations, verifications
│   ├── tracker.c              # Liste chainee des blocs
│   ├── guards.c               # Zones de garde
│   ├── inspection.c           # Statistiques et affichage
│   └── report.c               # Erreurs et fichier .log
├── tests/
│   ├── test.h                 # Macro CHECK, declarations
│   ├── runner.c               # Execute les 100 tests
│   ├── test_allocation.c      # 15 tests
│   ├── test_free.c            # 15 tests
│   ├── test_calloc_realloc.c  # 20 tests
│   ├── test_guards.c          # 10 tests
│   ├── test_inspection.c      # 20 tests
│   └── test_reports.c         # 20 tests
├── .gitignore
├── README.md
├── LICENSE                    # Licence MIT
└── Makefile
```

Les tests n'utilisent ni Criterion ni autre bibliotheque : chaque fonction
de test renvoie `1` si elle reussit et `0` sinon. Le runner les enregistre,
les execute et affiche un bilan. La commande `make test` echoue si l'un des
100 tests echoue.

## Limites : a lire avant d'utiliser

MyASAN est destine a **l'apprentissage**, pas a la verification de securite
d'un programme en production.

1. **Pas de surveillance generale des lectures/ecritures.** Un
   `use-after-free` n'est pas detecte automatiquement. Des ecritures qui
   sautent la garde ou qui sont corrigees avant verification peuvent passer.
2. **Pas d'interception globale.** Seuls les appels passant par les macros
   ou l'API sont suivis ; MyASAN n'utilise pas `LD_PRELOAD`.
3. **Pas de protection contre tous les pointeurs invalides.** Un pointeur
   interieur (milieu de bloc) n'est pas un pointeur suivi. La reutilisation
   d'une adresse par l'allocateur peut rendre un ancien pointeur ambigu.
4. **Pas de multithreading.** La liste chainee et les compteurs globaux ne
   sont pas proteges par des verrous.
5. **Les fiches des blocs liberes restent en memoire** jusqu'au `reset`.
6. **Ne pas melanger les allocateurs.** Ne pas appeler `free` natif sur un
   pointeur `MY_MALLOC`, ni `MY_FREE` sur un pointeur `malloc` natif.
7. Les appels `myasan_dump_blocks()` et `myasan_print_summary()` affichent
   directement sur stdout ; leur contenu n'est pas envoye au fichier log.

Pour detecter davantage d'erreurs dans un vrai programme C, utiliser
AddressSanitizer :

```sh
cc -g -fsanitize=address,undefined programme.c -o programme
./programme
```

## Licence

MIT, voir [`LICENSE`](LICENSE).
