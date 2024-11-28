#include <stdio.h>
#include <stdlib.h>

// Definição do grau mínimo da Árvore B
#define T 2

// Estrutura de um nó da Árvore B
typedef struct BTreeNode {
    int keys[2 * T - 1]; // Array de chaves
    struct BTreeNode *C[2 * T]; // Array de ponteiros para filhos
    int n; // Número atual de chaves
    int leaf; // Flag para indicar se é folha
} BTreeNode;

// Função para criar um novo nó da Árvore B
BTreeNode* createNode(int leaf) {
    BTreeNode* newNode = (BTreeNode*)malloc(sizeof(BTreeNode));
    newNode->leaf = leaf;
    newNode->n = 0;
    for (int i = 0; i < 2 * T; i++) {
        newNode->C[i] = NULL;
    }
    return newNode;
}

// Função para dividir um nó filho cheio y de um nó pai x
void splitChild(BTreeNode* x, int i, BTreeNode* y) {
    BTreeNode* z = createNode(y->leaf);
    z->n = T - 1;
    
    for (int j = 0; j < T - 1; j++) {
        z->keys[j] = y->keys[j + T];
    }

    if (!y->leaf) {
        for (int j = 0; j < T; j++) {
            z->C[j] = y->C[j + T];
        }
    }

    y->n = T - 1;

    for (int j = x->n; j >= i + 1; j--) {
        x->C[j + 1] = x->C[j];
    }

    x->C[i + 1] = z;

    for (int j = x->n - 1; j >= i; j--) {
        x->keys[j + 1] = x->keys[j];
    }

    x->keys[i] = y->keys[T - 1];
    x->n = x->n + 1;
}

// Função para inserir uma nova chave em uma Árvore B não cheia
void insertNonFull(BTreeNode* x, int k) {
    int i = x->n - 1;

    if (x->leaf) {
        while (i >= 0 && x->keys[i] > k) {
            x->keys[i + 1] = x->keys[i];
            i--;
        }
        x->keys[i + 1] = k;
        x->n = x->n + 1;
    } else {
        while (i >= 0 && x->keys[i] > k) {
            i--;
        }
        if (x->C[i + 1]->n == 2 * T - 1) {
            splitChild(x, i + 1, x->C[i + 1]);
            if (x->keys[i + 1] < k) {
                i++;
            }
        }
        insertNonFull(x->C[i + 1], k);
    }
}

// Função para inserir uma nova chave na Árvore B
void insert(BTreeNode** root, int k) {
    BTreeNode* r = *root;

    if (r->n == 2 * T - 1) {
        BTreeNode* s = createNode(0);
        *root = s;
        s->C[0] = r;
        splitChild(s, 0, r);
        insertNonFull(s, k);
    } else {
        insertNonFull(r, k);
    }
}

// Função auxiliar para obter o predecessor de uma chave
int getPred(BTreeNode* x, int idx) {
    BTreeNode* cur = x->C[idx];
    while (!cur->leaf) {
        cur = cur->C[cur->n];
    }
    return cur->keys[cur->n - 1];
}

// Função auxiliar para obter o sucessor de uma chave
int getSucc(BTreeNode* x, int idx) {
    BTreeNode* cur = x->C[idx + 1];
    while (!cur->leaf) {
        cur = cur->C[0];
    }
    return cur->keys[0];
}

// Função auxiliar para pegar emprestado de um irmão anterior
void borrowFromPrev(BTreeNode* x, int idx) {
    BTreeNode* child = x->C[idx];
    BTreeNode* sibling = x->C[idx - 1];

    for (int i = child->n - 1; i >= 0; --i) {
        child->keys[i + 1] = child->keys[i];
    }

    if (!child->leaf) {
        for (int i = child->n; i >= 0; --i) {
            child->C[i + 1] = child->C[i];
        }
    }

    child->keys[0] = x->keys[idx - 1];

    if (!x->leaf) {
        child->C[0] = sibling->C[sibling->n];
    }

    x->keys[idx - 1] = sibling->keys[sibling->n - 1];
    child->n += 1;
    sibling->n -= 1;
}

// Função auxiliar para pegar emprestado de um irmão próximo
void borrowFromNext(BTreeNode* x, int idx) {
    BTreeNode* child = x->C[idx];
    BTreeNode* sibling = x->C[idx + 1];

    child->keys[child->n] = x->keys[idx];

    if (!(child->leaf)) {
        child->C[child->n + 1] = sibling->C[0];
    }

    x->keys[idx] = sibling->keys[0];

    for (int i = 1; i < sibling->n; ++i) {
        sibling->keys[i - 1] = sibling->keys[i];
    }

    if (!sibling->leaf) {
        for (int i = 1; i <= sibling->n; ++i) {
            sibling->C[i - 1] = sibling->C[i];
        }
    }

    child->n += 1;
    sibling->n -= 1;
}

// Função auxiliar para mesclar o nó filho em idx com seu próximo irmão
void merge(BTreeNode* x, int idx) {
    BTreeNode* child = x->C[idx];
    BTreeNode* sibling = x->C[idx + 1];

    child->keys[T - 1] = x->keys[idx];

    for (int i = 0; i < sibling->n; ++i) {
        child->keys[i + T] = sibling->keys[i];
    }

    if (!child->leaf) {
        for (int i = 0; i <= sibling->n; ++i) {
            child->C[i + T] = sibling->C[i];
        }
    }

    for (int i = idx + 1; i < x->n; ++i) {
        x->keys[i - 1] = x->keys[i];
    }

    for (int i = idx + 2; i <= x->n; ++i) {
        x->C[i - 1] = x->C[i];
    }

    child->n += sibling->n + 1;
    x->n--;

    free(sibling);
}

// Função auxiliar para preencher um nó filho se ele tem menos que t chaves
void fill(BTreeNode* x, int idx) {
    if (idx != 0 && x->C[idx - 1]->n >= T) {
        borrowFromPrev(x, idx);
    } else if (idx != x->n && x->C[idx + 1]->n >= T) {
        borrowFromNext(x, idx);
    } else {
        if (idx != x->n) {
            merge(x, idx);
        } else {
            merge(x, idx - 1);
        }
    }
}

void deleteNode(BTreeNode* x, int k);

// Função para remover uma chave de um nó não folha
void removeFromNonLeaf(BTreeNode* x, int idx) {
    int k = x->keys[idx];

    if (x->C[idx]->n >= T) {
        int pred = getPred(x, idx);
        x->keys[idx] = pred;
        deleteNode(x->C[idx], pred);
    } else if (x->C[idx + 1]->n >= T) {
        int succ = getSucc(x, idx);
        x->keys[idx] = succ;
        deleteNode(x->C[idx + 1], succ);
    } else {
        merge(x, idx);
        deleteNode(x->C[idx], k);
    }
}

// Função para remover uma chave de um nó folha
void removeFromLeaf(BTreeNode* x, int idx) {
    for (int i = idx + 1; i < x->n; ++i) {
        x->keys[i - 1] = x->keys[i];
    }
    x->n--;
}

// Função para deletar uma chave da Árvore B
void deleteNode(BTreeNode* x, int k) {
    int idx = 0;
    while (idx < x->n && x->keys[idx] < k) {
        ++idx;
    }

    if (idx < x->n && x->keys[idx] == k) {
        if (x->leaf) {
            removeFromLeaf(x, idx);
        } else {
            removeFromNonLeaf(x, idx);
        }
    } else {
        if (x->leaf) {
            return;
        }

        int flag = (idx == x->n);
        if (x->C[idx]->n < T) {
            fill(x, idx);
        }

        if (flag && idx > x->n) {
            deleteNode(x->C[idx - 1], k);
        } else {
            deleteNode(x->C[idx], k);
        }
    }
}

// Função para remover uma chave da Árvore B
void delete(BTreeNode** root, int k) {
    if (*root == NULL) {
        return;
    }

    deleteNode(*root, k);

    if ((*root)->n == 0) {
        BTreeNode* tmp = *root;
        if ((*root)->leaf) {
            *root = NULL;
        } else {
            *root = (*root)->C[0];
        }
        free(tmp);
    }
}

// Função para buscar uma chave na Árvore B
BTreeNode* search(BTreeNode* x, int k, int* index) {
    int i = 0;
    while (i < x->n && k > x->keys[i]) {
        i++;
    }
    if (i < x->n && k == x->keys[i]) {
        *index = i;
        return x;
    } else if (x->leaf) {
        return NULL;
    } else {
        return search(x->C[i], k, index);
    }
}

// Função para imprimir a Árvore B no formato desejado
void printTree(BTreeNode* root, int level) {
    if (root != NULL) {
        for (int i = 0; i < root->n; i++) {
            printTree(root->C[i], level + 1);
            for (int j = 0; j < level; j++) {
                printf("  ");
            }
            printf("> %d\n", root->keys[i]);
        }
        printTree(root->C[root->n], level + 1);
    }
}

int main() {
    BTreeNode* root = createNode(1);

    int values[] = {7, 8, 9, 10, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
    int n = sizeof(values) / sizeof(values[0]);

    for (int i = 0; i < n; i++) {
        insert(&root, values[i]);
    }

    printTree(root, 0);

    int index;
    BTreeNode* result = search(root, 15, &index);
    if (result != NULL) {
        printf("Chave %d encontrada no índice %d\n", result->keys[index], index);
    } else {
        printf("Chave não encontrada\n");
    }

    delete(&root, 15);
    printTree(root, 0);

    return 0;
}