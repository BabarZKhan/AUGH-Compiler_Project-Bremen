/**
 * ***********************************************************************
 * Papillon: ensemble de recherche distribué et multi-critères
 *
 * Auteur: Ivan auge (ivan.auge@ensiie.fr)
 * ***********************************************************************
**/

/*
 * ***********************************************************************
 * I)   Fichiers
 * II)  Schéma général
 * III) Fonction générale
 * IV)  Détail des requêtes
 * V)   Opérateurs
 * ***********************************************************************
 * I) Fichiers et compilation
 *
 *   Papillon:
 *   - README.h: définition des commandes et de leur formats.
 *   - config.h: définition des paramètres structuraux du papillon
 *   - papillon.h xquery.h pap-node-leaf.c xquery.c: une implémentation
 *               C des commandes.
 *   - pap-node.c: le papillon noeud.
 *   - pap-leaf.c: le papillon feuille.
 *
 *   Simulateur:
 *    - simu.c: un simulateur qui crée un arbre de papillon communiquant
 *      par des pipes.
 *    - simu-interface.c: l'interface textuelle de simulateur.
 *    - simu.dat: une liste de commande au format de simu-interface.c
 *
 *   Compilation 
 *    $ gcc pap-node.c pap-leaf.c xquery.c simu.c simu-interface.c
 *
 *   Compilation avec la bibliothèque readline
 *    $ gcc -DREADLINE pap-leaf.c pap-node.c xquery.c simu-interface.c simu.c \
 *       -lreadline -lhistory -lcurses
 *
 *   Test interactif avec 3 papillons
 *       $ ./a.out 3
 *       ./a.out: pap_ node-processor is started
 *       ./a.out: pap_R leaf-processor is started
 *       ./a.out: pap_L leaf-processor is started
 *       cmd (? for help) --> add aaaaa 100
 *       Add is completed: free rate=  0
 *       cmd (? for help) --> exit
 *       FATAL:papillon_node: read failure, exited
 *       FATAL:papillon_leaf: read failure, exited
 *       FATAL:papillon_leaf: read failure, exited
 *       $
 *
 *   Test des commandes de simu.dat avec un arbre de 5 papillons
 *       $ echo l | ./a.out 5
 *   ou
 *       $ echo l simu.dat | ./a.out 5
 *   ou
 *       $ ./a.out 5 < simu.dat
 *
 * ***********************************************************************
 * II) Schéma général
 *
 * Les papillons sont conçus pour former des arbres binaires.
 *
 *                            +------+
 *                            |      |
 *                      +---->|      |
 *                      |     | leaf |
 *           +------+   | +---|      |         +------+
 *  Entrée   |      |---+ |   |      |         |      |
 *       --->|      |<----+   +------+   +---->|      |              
 *           | node |                    |     | leaf |             
 *       <---|      |-----+   +------+   | +---|      |
 *  Sortie   |      |<--+ |   |      |---+ |   |      |
 *           +------+   | +-->|      |<----+   +------+      
 *                      |     | node |                 
 *                      +-----|      |-----+   +------+
 *                            |      |<--+ |   |      |
 *                            +------+   | +-->|      |
 *                                       |     | leaf |
 *                                       +-----|      |
 *                                             |      |
 *                                             +------+
 *
 * a) Les papillons feuilles ont 1 flux d'entrée et un flux de sortie.
 * Le papillon lit une commande sur ce flux d'entrée , la traite et renvoie
 * la réponse à cette commande sur le flux de sortie.
 * b) Les papillons noeuds ont en plus 1 flux d'entrée et un flux de sortie vers
 * le fils gauche et le fils droit. Ils traitent les commandes qu'ils reçoivent
 * comme les papillons feuille mais en plus diffusent les commandes à leurs
 * fils et les réponses de leurs fils à leur père.
 *
 * ***********************************************************************
 * III) Fonction générale
 *
 * Les papillons sont une implémentation d'un ensemble de recherche multi-
 * critère (il est même possible d'en gérer plusieurs simultanément).
 * De ce fait, c'est un élément de mémorisation d'enregistrements.
 * Un enregistrement est composé d'un index (32 bits), et de 4*n octets de données
 * (n dans [1:MAX_ELESIZE]).
 *
 * La requête "ADD(index,data)" ajoute l'enregistrement à
 * l'ensemble. L'ajout est fait de manière à répartir équitablement les
 * enregistrements sur les papillons. La réponse à cette requête
 * est le taux minimal d'occupation mémoire des papillons.
 * 
 * Les requêtes "QUERY(critéres)" renvoient les index des enregistrements
 * dont les données correspondent aux critères.
 *
 * ***********************************************************************
 * V) Détail des requêtes
 *
 * Le format général des requêtes comme des réponses est:
 *   - un mot de 32 bits avec en poids fort (sur 4 bits) un code opération (CODOP),
 *   - suivi de zéro ou plusieurs mots de 32 bits.
 * Par la suite on emploiera "mot" pour mot de 32 bits.
 *
 * La gestion des requêtes est séquentielle, une deuxième requête n'est traitée
 * que quand la réponse à la première requête a été délivrée.
 *
 * Requête CMD_ADD_SHORT:
 *
 *   Elle ajoute un enregistrement (index,données) à l'ensempble.
 *   
 *   Elle est constituée d'un mot contenant CMD_ADD_SHORT+dtsz+index
 *   (voir macro CMD_SET_CODOP_DTSZ_VALUE) suivi de dtsz mots qui contiennent
 *   les données de l'enregistrement.
 *
 *   La réponse est un mot CMD_ADD_STATUS+freerate (voir macros CMD_SET_CODOP_FREERATE,
 *   CMD_CODOP, CMD_FREERATE)
 *   Le freerate est un naturel de 8 bits qui donne le taux d'occupation mémoire
 *   du papillon le moins rempli. 0 signifie vide, 255 signifie plein.
 *
 *   Pré-requis:
 *      - Dictionnaire non plein
 *      - 0 < dtsz <= MAX_ELESIZE
 *
 * Requête CMD_ADD_LONG:
 *
 *   Elle est constituée d'un mot contenant CMD_ADD_SHORT+dtsz (voir macro
 *   CMD_SET_CODOP_DTSZ), suivi d'un mot contenant l'index, suivi dtsz
 *   mots qui contiennent les données de l'enregistrement.
 *
 *   La fonction, la réponse et les pré-requis sont les mêmes que CMD_ADD_SHORT.
 *
 *
 * Requête CMD_QUERY:
 *
 *   Soit une clé (pos,key) où pos est une position et data n caractères,
 *   elle recherche tous les enregistrements (index,data) de l'ensemble dont
 *   les caractères data[4*pos:4*pos+n-1] correspondent à la clé key[0:n-1].
 *   et elle renvoie leurs index.
 *   
 *   data[4*pos:4*pos+n-1] et key[0:n-1] correspondent si quelques soit i dans [0:n[,
 *   key[i] est égal à 0xFF où data[4*pos+i] et key[i] sont égaux.
 *
 *   Le format de la requête est constituée d'un mot contenant CMD_QUERY+dtsz+pos
 *   (voir macro CMD_SET_CODOP_DTSZ_VALUE) suivi de dtsz mots qui contiennent
 *   les données de la clé (les key[i]).
 *
 *   La réponse est:
 *     - zéro ou plusieurs CMD_RESULT+index (voir macros CMD_CODOP, CMD_LONGVAL)
 *     - suivi d'un mot CMD_END (voir macro CMD_CODOP),
 *       ce mot indique la fin de la requête.
 *
 *   Pré-requis:
 *      - 0 <  dtsz <= MAX_ELESIZE
 *      - 0 <= pos  <  MAX_ELESIZE-1
 *   
 *   Exemple:
 *      - recherche de tous les enregistrements commençant par "jo":
 *          CMD_SET_CODOP_DTSZ_VALUE(CMD_QUERY,1,0) jo\377\377
 *      - recherche de tous les enregistrements dont les caractères 6, 7, 8 et 9
 *        sont 'j', 'o', 'h', 'n':
 *          CMD_SET_CODOP_DTSZ_VALUE(CMD_QUERY,2,1) \377\377jo hn\377\377
 *
 * Requête CMD_XQUERY:
 *
 *   Un opérateur est une fonction qui appliquée sur un enregistrement,
 *   renvoie un booleen. Les opérateurs disponibles sont définis dans le
 *   chapitre suivant.
 *      operateur: Ensemble d'enregistrements---> Ensemble des booléens
 *
 *   La requête CMD_XQUERY permet de définir une fonction logique F
 *   d'opérateurs comme par exemple
 *     F(e) = { opérateur0(e) || opérateur1(e) } &&
 *            { opérateur2(e) || opérateur3(e) }
 *   où e est un enregistrement. Elle recherche tous les enregistrements
 *   e=(index,data) de l'ensemble tel que F(e) soit vrai et elle renvoie
 *   leurs index.
 *
 *   Une requête XQUERY est composée de 1 ou plusieurs requêtes
 *   CMD_XQUERY+so+lo+end+nbop (voir macro CMD_SET_CODOP_SO_LO_END_NBOP)
 *   suivi de nbop opérateurs.
 *     - lo est un opérateur logique naire (ET OU ... voir les macros CMD_LOGOP_xxx).
 *       Un enregistrement e est sélectionné si
 *           opérateur0(e) lo opérateur1(e) lo opérateur2(e) ...
 *       est vrai.
 *     - so est un opérateur ensembliste (voir les macros CMD_SETOP_xxx).
 *         * so=UNION: l'ensemble est parcouru et les enregistrements sélectionnés
 *           sont ajoutés à WS (un ensemble d'enregistrements temporaire).
 *         * so=INTER: WS est parcouru et les enregistrements non selectionés
 *           sont enlevés de WS.
 *     - end est un booléen, la valeur 1 (vrai) indique que c'est la dernière
 *       requête et que le papillon doit envoyer les résultats.
 *     - nbop est le nombre d'opérateurs.
 *
 *    La réponse est similaire à celle de la requête CMD_QUERY.
 *
 *   Pré-requis:
 *      - 0 <  nbop <= OPE_NB
 *      - + pré-requis des opérateurs
 *
 *    Exemple: La fonction F définie ci-dessus peut être représentée par:
 *      CMD_SET_CODOP_SO_LO_END_NBOP(CMD_XQUERY,CMD_SETOP_UNION,CMD_LOGOP_AND,0,2) opérateur0 opérateur1
 *      CMD_SET_CODOP_SO_LO_END_NBOP(CMD_XQUERY,CMD_SETOP_INTER,CMD_LOGOP_AND,1,2) opérateur2 opérateur3
 *
 * ***********************************************************************
 * IV) Opérateurs
 *
 * Les opérateurs sont:
 *   - In character subset : détermine si les caractères d'un champs sont
 *       compris dans un ensemble de caractères
 *   - Match Regular expression : détermine si les caractères d'un champs
 *       correspondent à une expression régulière.
 *   - Lexical order : compare suivant l'ordre lexicographique les caractères
 *       d'un champs à une chaine de caractères.
 *   - BCD order : compare suivant l'ordre numérique les caractères d'un
 *       champs représentant un entier en BCD à un entier.
 *   - hexa order : compare suivant l'ordre numérique les caractères d'un
 *       champs représentant un entier non signé en héxadécimal
 *       à un entier non signé.
 *   - Binary order : compare suivant l'ordre numérique les caractères
 *       d'un champs représentant un entier en binaire à un entier.
 * 
 * +------+-------+----------------------------------------+
 * | bits | fields|         operators                      |
 * +------+-------+----------------------------------------+
 * |31-28 |       | IN |REGEXPR|STRCMP|BCDCMP|HEXCMP|BINCMP|
 * +------+-------+----------------------------------------+
 * |27-24 | flag  | *  | *     |  *   |  *   |  *   |  *   |
 * |      |    ci | *  | *     |  *   |      |  *   |      |
 * |      |   top | *  | *     |  *   |  *   |  *   |      |
 * |      |   bot | *  | *     |  *   |  *   |  *   |      |
 * |      |    le |    |       |      |      |      |  *   |
 * +------+-------+----------------------------------------+
 * |23-20 |e cmp o| 0  | 0     |  *   |  *   |  *   |  *   |
 * |      |  ==   |    |       |  *   |  *   |  *   |  *   |
 * |      |  !=   |    |       |  *   |  *   |  *   |  *   |
 * |      |  >[=] |    |       |  *   |  *   |  *   |  *   |
 * |      |  <[=] |    |       |  *   |  *   |  *   |  *   |
 * +------+-------+----------------------------------------+
 * |19-10 | from  | *  | *     |  *   |  *   |  *   |  *   |
 * | 9- 4 | len   | *  | *     |  *   |  *   |  *   |  *   |
 * | 3- 0 | dtlen | *  | *     |  *   |  *   |  0   |  1   |
 * +------+-------+----+-------+------+------+------+------+
 *    ci : case insensitive, in this case operator data must be
 *         in lower case.
 *    top: the trailing space or zero of record data are ignored.
 *    bot: the leading space or zero of record data are ignored.
 *    le : use little endian byte order for binary data (default
 *         is big endian)
 *    e cmp o: compare les données de l'enregistrement aux données
 *         de l'operateur.
 *
 * OPE_FCT_IN
 *   Les données de l'opérateur définissent un ensemble de couples (l(i),r(i))
 *   de caractères. Si le dernier couple est (0,0), il est ignorés.
 *   Soit C(j) les len caractères de l'enregistrement (j dans [from,from+len[),
 *   Soit les fonctions booléenn B(j) définies par:
 *     - B(j) est vrai si il existe un i tel que: l(i) <= c(j) <= r(i),
 *   L'enregistrement est sélectionné si un B(j) est vrai.
 *
 * OPE_FCT_REGEXPR
 *   Les données de l'opérateur définissent une chaine de caractères, si elle se
 *   termine par 1, 2 ou 3 zéro, ceux si son ignorés.
 *   Dans cette chaine de caractères les caractères suivant sont interprétés:
 *     - '.': 1 caractère de n'importe quelle valeur.
 *     - '?': 0 ou 1 caractère de n'importe quelle valeur.
 *     - '+': au moins 1 caractères de n'importe quelle valeur.
 *     - '*': 0 ou plusieurs caractères de n'importe quelle valeur.
 *   L'enregistrement est sélectionné si les données correspondent à
 *   l'expression régulière.
 *
 * OPE_FCT_STRCMP
 *   Les données de l'opérateur définissent une chaine de caractères, si elle se
 *   termine par 1, 2 ou 3 zéro, ceux si son ignorés.
 *   La chaine de caractères de l'enregistrement est comparée suivant
 *   l'ordre lexicographique à la chaine de caractère de l'opérateur.
 *   L'enregistrement est sélectionné si l'opérateur cmp correspond.
 *
 * OPE_FCT_BCDCMP
 *   Les données de l'opérateur définissent une chaine de caractères, si elle se
 *   termine par 1, 2 ou 3 zéro, ceux si son ignorés.
 *   Cette chaine de caractères ainsi que les données de l'enregistrement
 *   doivent représentées des nombres entiers (dans +-.0-9).
 *   L'entier de l'enregistrement est comparé à l'entier de l'opérateur.
 *   L'enregistrement est sélectionné si l'opérateur cmp correspond.
 *
 * OPE_FCT_HEXCMP
 *   Les données de l'opérateur définissent une chaine de caractères, si elle se
 *   termine par 1, 2 ou 3 zéro, ceux si son ignorés.
 *   Cette chaine de caractères ainsi que les données de l'enregistrement
 *   doivent représentées des nombres naturels codé en hexadécimal (dans
 *   [0-9] ou [A-F]).
 *   Le naturel de l'enregistrement est comparé au naturel de l'opérateur.
 *   L'enregistrement est sélectionné si l'opérateur cmp correspond.
 *
 * OPE_FCT_BINCMP
 *   Les données de l'opérateur définissent un nombre binaire en complément à 2
 *   de 1 (suivi de 3 zéro), 2 (suivi de 2 zéro) ou 4 octets.
 *   Les données de l'opérateur définissent aussi un nombre binaire en complément à 2
 *   de 1 (suivi de 3 zéro), 2 (suivi de 2 zéro) ou 4 octets.
 *   L'entier de l'enregistrement est comparé à l'entier de l'opérateur,
 *   L'enregistrement est sélectionné si l'opérateur cmp correspond.
 * ***********************************************************************
**/

#ifndef FILE_README_H
#define FILE_README_H

/**
 * ***********************************************************************
 * Command and request definition
**/

// Operation code
#define CMD_ADD_SHORT  1    // format: CODOP,DATA-SIZE,VALUE + DATA-SIZE words
#define CMD_ADD_LONG   2    // format: CODOP,DATA-SIZE,0     + value word + DATA-SIZE words
#define CMD_ADD_STATUS 3    // format: CODOP,rate,0 
#define CMD_QUERY      4    // format: CODOP,DATA-SIZE,0     + DATA-SIZE words
#define CMD_RESULT     5    // format: CODOP,value
#define CMD_END        6    // format: CODOP,0
#define CMD_XQUERY     7    // format: CODOP,so,lo,end,nbop

// Macro for retrieving fields
#define CMD_CODOP(x)           (((x)>>28)&0xF)
#define CMD_KEYSZ(x)           (((x)>>16)&0xFFF)
#define CMD_SHORTVAL(x)        ((x)&0xFFFF)
#define CMD_LONGVAL(x)         ((x)&0xFFFFFFF)
#define CMD_FREERATE(x)        ((x)&0x00FF)
#define CMD_SETOP(x)           (((x)>>24)&0x03)
#define CMD_LOGOP(x)           (((x)>>20)&0x0F)
#define CMD_COMMIT(x)          (((x)>>16)&0x01)
#define CMD_NBOPE(x)           (((x)>>12)&0x0F)

// Macro for building the main word of request or command
#define CMD_SET_CODOP(cmd)               (((cmd)<<28)&0xF0000000)
#define CMD_SET_CODOP_DTSZ(cmd,ksz)      CMD_SET_CODOP(cmd) | (((ksz)<<16)&0x0FFF0000)
#define CMD_SET_CODOP_DTSZ_VALUE(cmd,ksz,val) \
                                         CMD_SET_CODOP_DTSZ(cmd,ksz) \
                                                            | ((val)&0x0000FFFF)
#define CMD_SET_CODOP_FREERATE(cmd,fr)   CMD_SET_CODOP(cmd) | ((fr)&0x0000FFFF)
#define CMD_SET_CODOP_VALUE(cmd,value)   CMD_SET_CODOP(cmd) | ((value)&0x0FFFFFFF)
#define CMD_SET_CODOP_SO_LO_END_NBOP(cmd,so,lo,cmt,nbo) \
                                        (CMD_SET_CODOP(cmd) | ((( so)&0x03)<<24) | ((( lo)&0x0F)<<20) \
                                                            | (((cmt)&0x01)<<16) | (((nbo)&0x0F)<<12))

/**
 * ***********************************************************************
 * Operator definition
**/

#define OPE_FCT(x)   (((x)>>28)&0x0000000F)
#define OPE_FLAG(x)  (((x)>>24)&0x0000000F)
#define OPE_CMP(x)   (((x)>>20)&0x0000000F)
#define OPE_FROM(x)  (((x)>>10)&0x000003FF)
#define OPE_LEN(x)   (((x)>> 4)&0x0000003F)
#define OPE_DTLEN(x) (((x)>> 0)&0x0000000F)

#define OPE_SET_FCT(x)    (((x)<<28)&0xF0000000)
#define OPE_SET_FLAG(x)   (((x)<<24)&0x0F000000)
#define OPE_SET_CMP(x)    (((x)<<20)&0x00F00000)
#define OPE_SET_FROM(x)   (((x)<<10)&0x000FFC00)
#define OPE_SET_LEN(x)    (((x)<< 4)&0x00F003F0)
#define OPE_SET_DTLEN(x)  (((x)<< 0)&0x00F0000F)
#define OPE_SET_FCT_FL_CMP_FROM_LEN_DTLEN(fct,fl,cmp,from,len,dtl) \
    OPE_SET_FCT(fct)   | OPE_SET_FLAG(fl) | OPE_SET_CMP(cmp)    | \
    OPE_SET_FROM(from) | OPE_SET_LEN(len) | OPE_SET_DTLEN(dtl)

#define OPE_FCT_FALSE     1
#define OPE_FCT_TRUE      2
#define OPE_FCT_IN        3
#define OPE_FCT_REGEXPR   4
#define OPE_FCT_STRCMP    5
#define OPE_FCT_BCDCMP    6
#define OPE_FCT_HEXCMP    7
#define OPE_FCT_BINCMP    8

#define OPE_FLAG_CI       1
#define OPE_FLAG_TOP      2
#define OPE_FLAG_BOT      4
#define OPE_FLAG_LE       8
#define OPE_FLAG_IsCI(x)  (OPE_FLAG(x)&OPE_FLAG_CI)
#define OPE_FLAG_IsTOP(x) (OPE_FLAG(x)&OPE_FLAG_TOP)
#define OPE_FLAG_IsBOT(x) (OPE_FLAG(x)&OPE_FLAG_BOT)
#define OPE_FLAG_IsLE(x)  (OPE_FLAG(x)&OPE_FLAG_LE)

// logic operators
#define OPLOG_AND 1
#define OPLOG_OR  2

// operators about set
#define OPSET_UNION 1
#define OPSET_INTER 2

#endif
