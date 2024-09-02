#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define TRUE 1
#define FALSE 0
#define EMPTY 0

#define NODE_ORDER		501
#define NODE_POINTERS	(NODE_ORDER*2)
#define NODE_KEYS		NODE_POINTERS-1

typedef unsigned char bool;

int inseridosMat = 0;
int inseridosRg = 0;
int tamTotal;

typedef struct tree_node {
	int key_array[NODE_KEYS];
	struct tree_node *child_array[NODE_POINTERS];
	unsigned int key_index;
    long int tamanho[NODE_KEYS];
	bool leaf;
} node_t;

typedef struct {
	long int node_pointer;
	int key;
	bool found;
	unsigned int depth;
} result_t;


typedef struct {
	node_t *root;
	unsigned short order;
	bool lock;
} btree_t;

static int BTreeGetLeftMax(node_t *T);
static int BTreeGetRightMin(node_t *T);

static node_t *create_node(int id){
	FILE *file;
	FILE *filee;
	file = fopen("arquivoId.dat","ab");
	filee = fopen("arquivoRg.dat","ab");
	int i;
	long int tamPagina, tamPaginaa;
	node_t *new_node = (node_t *)malloc(sizeof(node_t));

	if(!new_node){
		printf("Out of memory");
		exit(0);
	}

	for(i = 0;i < NODE_KEYS; i++){
		new_node->key_array[i] = 0;
	}

	for(i = 0;i < NODE_POINTERS; i++){
		new_node->child_array[i] = NULL;
	}

	new_node->key_index = EMPTY;
	new_node->leaf = TRUE;

	if(id == 0){ //arquivo ID
		tamPagina = ftell(file);
		fwrite(new_node,sizeof(node_t),1,file);
	}else{
		tamPaginaa = ftell(filee);
		fwrite(new_node,sizeof(node_t),1,filee);
	}

	return new_node;
}

btree_t *create_btree(int id){
	btree_t *new_root = (btree_t *)malloc(sizeof(btree_t));

	if(!new_root){
		return NULL;
	}

	node_t *head = create_node(id);

	if(!head){
		return NULL;
	}

	new_root->order = NODE_ORDER;
	new_root->root = head;
	new_root->lock = FALSE;

	return new_root;
}

static result_t *get_resultset(){
	result_t *ret = (result_t *)malloc(sizeof(result_t));

	if(!ret){
		printf("ERROR! Out of memory.");
		exit(0);
	}

	ret->key = 0;
	ret->found = FALSE;
	ret->depth = 0;

	return ret;
}

void print_node(node_t *n){
	int i, q;

	printf("  Index: %d\n", n->key_index);
	printf("   Leaf: ");
	if(n->leaf){
		printf("TRUE\n");
	}else{
		printf("FALSE\n");
	}

	printf("  Array:");
	for(i = 0; i < n->key_index; i++){
		printf(" [%d : %d]", i, n->key_array[i]);
	}

	printf("\n  Child:");
	if(n->leaf){
		printf(" NONE");
	}else{
		for(q = 0; q < NODE_POINTERS; q++){
		}
	}

	printf("\n<<------------------\n");
}

result_t *search(int key, node_t *node){
	int i = 0;

	while((i < node->key_index) && (key > node->key_array[i])){
		i++;
	}

	if(i == NODE_KEYS){
		i--;
	}

	if((i <= node->key_index) && (key == node->key_array[i])){
		result_t *result = get_resultset();
		result->node_pointer = node->tamanho[i]; //recebe node->tamanho[i]
		result->key = i;
		result->found = TRUE;
    	printf("A chave %d está contida na árvore.\n", node->key_array[i]);

    	printf("A posição em bytes é = %ld\n", node->tamanho[i]);

		return result;
	}

	if(node->leaf){
		result_t *result = get_resultset();
		result->node_pointer = node->tamanho[i];
		result->found = FALSE;
		return result;
	}else{
		result_t *result = get_resultset();
		return search(key, node->child_array[i]);
	}
}

static void split_child(node_t *parent_node, int i, node_t *child_array, int id){
	int j;

	node_t *new_node = create_node(id);
	new_node->leaf = child_array->leaf;
	new_node->key_index = NODE_ORDER-1;

	for(j = 0;j < NODE_ORDER-1;j++){
		new_node->key_array[j] = child_array->key_array[NODE_ORDER+j];
    	new_node->tamanho[j] = child_array->tamanho[NODE_ORDER+j];
	}

	if(child_array->leaf == 0){
		for(j = 0;j < NODE_ORDER;j++){
			new_node->child_array[j] = child_array->child_array[NODE_ORDER+j];
		}
	}
	child_array->key_index = NODE_ORDER-1;

	for(j = parent_node->key_index;j>=i;j--){
		parent_node->child_array[j+1] = parent_node->child_array[j];
	}

	parent_node->child_array[i] = new_node;

	for(j = parent_node->key_index;j>=i;j--){
		parent_node->key_array[j] = parent_node->key_array[j-1];
    	parent_node->tamanho[j]=parent_node->tamanho[j-1];
	}

	parent_node->key_array[i-1] = child_array->key_array[NODE_ORDER-1];
    parent_node->tamanho[i-1] = child_array->tamanho[NODE_ORDER-1];

	parent_node->key_index++;
}

static void insert_nonfull(node_t *n, int key, long int tamanho, int id){
	int i = n->key_index;

	if(n->leaf){
		while(i>=1 && key<n->key_array[i-1]){
			n->key_array[i] = n->key_array[i-1];
            n->tamanho[i]=n->tamanho[i-1];
			i--;
		}

		n->key_array[i] = key;

        n->tamanho[i] = tamanho;
        n->key_index++;
        inseridosMat++;

	}else{
		while(i>=1 && key<n->key_array[i-1]){
			i--;
		}
		if(n->child_array[i]->key_index == NODE_KEYS){
			split_child(n, i+1, n->child_array[i], id);
			if(key > n->key_array[i]){
				i++;
			}
		}
		insert_nonfull(n->child_array[i], key, tamanho, id);
	}
}

node_t *insert(int key, btree_t *b, long int tamanho, int id){
	if(!b->lock){
		node_t *root = b->root;
		if(root->key_index == NODE_KEYS){
			node_t *newNode = create_node(id);
			b->root = newNode;
			newNode->leaf = 0;
			newNode->key_index = 0;
			newNode->child_array[0] = root;
			split_child(newNode, 1, root, id);
			insert_nonfull(newNode, key, tamanho, id);
		}else{
			insert_nonfull(b->root, key, tamanho, id);
		}
	}else{
		printf("Tree is locked\n");
	}

	return b->root;
}

static void merge_children(node_t *root, int index, node_t *child1, node_t *child2){
	child1->key_index = NODE_KEYS;
	int i;

	for(i=NODE_ORDER;i<NODE_KEYS;i++){
		child1->key_array[i] = child2->key_array[i-NODE_ORDER];
    child1->tamanho[i] = child2->tamanho[i-NODE_ORDER];
  }
	child1->key_array[NODE_ORDER-1] = root->key_array[index];
  child1->tamanho[NODE_ORDER-1] = root->tamanho[index];


	if(0 == child2->leaf){
		for(i=NODE_ORDER;i<NODE_POINTERS;i++)
			child1->child_array[i] = child2->child_array[i-NODE_ORDER];
	}

	for(i=index+1;i<root->key_index;i++){
		root->key_array[i-1] = root->key_array[i];
		root->child_array[i] = root->child_array[i+1];
    root->tamanho[i-1] = root->tamanho[i];
	}
	root->key_index--;
	free(child2);
}

static void BTreeBorrowFromLeft(node_t *root, int index, node_t *leftPtr, node_t *curPtr){
	curPtr->key_index++;
	int i;
	for(i=curPtr->key_index-1;i>0;i--){
		curPtr->key_array[i] = curPtr->key_array[i-1];
    curPtr->tamanho[i] = curPtr->tamanho[i-1];
  }
	curPtr->key_array[0] = root->key_array[index];
  curPtr->tamanho[0] = root->tamanho[index];
	root->key_array[index] = leftPtr->key_array[leftPtr->key_index-1];
  root->tamanho[index] = leftPtr->tamanho[leftPtr->key_index-1];
	if(0 == leftPtr->leaf){
		for(i=curPtr->key_index;i>0;i--)
			curPtr->child_array[i] = curPtr->child_array[i-1];
	}
	curPtr->child_array[0] = leftPtr->child_array[leftPtr->key_index];
	leftPtr->key_index--;
}

static void BTreeBorrowFromRight(node_t *root, int index, node_t *rightPtr, node_t *curPtr){
	curPtr->key_index++;
	curPtr->key_array[curPtr->key_index-1] = root->key_array[index];
  curPtr->tamanho[curPtr->key_index-1] = root->tamanho[index];
	root->key_array[index] = rightPtr->key_array[0];
  root->tamanho[index] = rightPtr->tamanho[0];
	int i;
	for(i=0;i<rightPtr->key_index-1;i++)
		rightPtr->key_array[i] = rightPtr->key_array[i+1];
    rightPtr->tamanho[i] = rightPtr->tamanho[i+1];
	if(0 == rightPtr->leaf){
		curPtr->child_array[curPtr->key_index] = rightPtr->child_array[0];
		for(i=0;i<rightPtr->key_index;i++)
			rightPtr->child_array[i] = rightPtr->child_array[i+1];

	}
	rightPtr->key_index--;
}

static void BTreeDeleteNoNone(int X, node_t *root){
	int i;
	if(1 == root->leaf){
		i=0;
		while( (i<root->key_index) && (X>root->key_array[i]))
			i++;
		if(X == root->key_array[i]){
			for(;i<root->key_index-1;i++)
				root->key_array[i] = root->key_array[i+1];
        root->tamanho[i] = root->tamanho[i+1];
			root->key_index--;
		}
		else{
			printf("Node not found.\n");
			return ;
		}
	}
	else{
		i = 0;
		node_t *prePtr = NULL, *nexPtr = NULL;
		//Find the index;
		while( (i<root->key_index) && (X>root->key_array[i]) )
			i++;
		if( (i<root->key_index) && (X == root->key_array[i]) ){
			prePtr = root->child_array[i];
			nexPtr = root->child_array[i+1];

			if(prePtr->key_index > NODE_ORDER-1){
				int aPrecursor = BTreeGetLeftMax(prePtr);
				root->key_array[i] = aPrecursor;
                root->tamanho[i] = aPrecursor;
				BTreeDeleteNoNone(aPrecursor,prePtr);
			}
			else
			if(nexPtr->key_index > NODE_ORDER-1){
				int aSuccessor = BTreeGetRightMin(nexPtr);
				root->key_array[i] = aSuccessor;
                root->tamanho[i] = aSuccessor;
				BTreeDeleteNoNone(aSuccessor,nexPtr);
			}
			else{
				merge_children(root,i,prePtr,nexPtr);
				BTreeDeleteNoNone(X,prePtr);
			}
		}
		else{
			prePtr = root->child_array[i];
			node_t *leftBro = NULL;
			if(i<root->key_index)
				nexPtr = root->child_array[i+1];
			if(i>0)
				leftBro = root->child_array[i-1];
			if(NODE_ORDER-1 == prePtr->key_index){
				if( (leftBro != NULL) && (leftBro->key_index > NODE_ORDER-1))
					BTreeBorrowFromLeft(root,i-1,leftBro,prePtr);
				else
				if( (nexPtr != NULL) && (nexPtr->key_index > NODE_ORDER-1))
					BTreeBorrowFromRight(root,i,nexPtr,prePtr);

				else if(leftBro != NULL){

					merge_children(root,i-1,leftBro,prePtr);
					prePtr = leftBro;
				}
				else
					merge_children(root,i,prePtr,nexPtr);
			}
			BTreeDeleteNoNone(X,prePtr);
		}
	}
}

static int BTreeGetLeftMax(node_t *T){
	if(0 == T->leaf){
		return BTreeGetLeftMax(T->child_array[T->key_index]);
	}else{
		return T->key_array[T->key_index-1];
	}
}

static int BTreeGetRightMin(node_t *T){
	if(0 == T->leaf){
		return BTreeGetRightMin(T->child_array[0]);
	}else{
		return T->key_array[0];
	}
}

node_t *delete(int key, btree_t *b){
	if(!b->lock){
		if(b->root->key_index == 1){
			node_t *child1 = b->root->child_array[0];
			node_t *child2 = b->root->child_array[1];
			if((!child1) && (!child2)){
				if((NODE_ORDER-1 == child1->key_index) && (NODE_ORDER-1 == child2->key_index)){
					merge_children(b->root, 0, child1, child2);
					free(b->root);
					BTreeDeleteNoNone(key, child1);
					return child1;
				}
			}
		}
		BTreeDeleteNoNone(key, b->root);
	}else{
		printf("Tree is locked\n");
	}
	return b->root;
}

void tree_unlock(btree_t *r){
	r->lock = FALSE;
}

bool tree_lock(btree_t *r){
	if(r->lock){
		return FALSE;
	}
	r->lock = TRUE;

	return TRUE;
}

void GravarDados(node_t *chave, char arquivo[]){
    FILE *file;
    file = fopen(arquivo, "ab");
    if(file == NULL){
        printf("\nErro ao abrir o arquivo.");
    }else{
        if(fwrite(chave, sizeof(node_t), 1, file) == 0){
            printf("\nErro ao inserir dados no arquivo.");
        }else{
            printf("\nDados inseridos com sucesso!");
        }
    }

    fclose(file);
}

int main(int argc, char *argv[]){
	btree_t *b = create_btree(0);
  btree_t *bb = create_btree(1);
  int id, rg, idade, busca = -1, i, buscar;
  char nome[20], departamento[50], caracter;
  long int tamanho, bytes, tamanhoInsercao;
  int flag = 0;
  double salario;

	long int tamCabecalho, tamCabecalhoo, paginas[100];
	long int tamPai, tamanhoAux, paiJafoi = 0;

  FILE *arq;
  arq = fopen("arq.txt","r");
  result_t *rs;

	FILE *file;
	file = fopen("arquivoId.dat","ab");

	FILE *filee;
	filee = fopen("arquivoRg.dat","ab");

	tamCabecalho = ftell(file);
	fwrite(b, sizeof(btree_t),1,file);

	tamCabecalhoo = ftell(filee);
	fwrite(bb, sizeof(btree_t),1,filee);

    if(arq){
        while(!feof(arq)){
            fflush(stdin);
            tamanho = ftell(arq);

            i = fscanf(arq,"%d %s %d %lf %d %s %c",&id, nome, &rg, &salario, &idade, departamento, &caracter);

            if(i == 7){
                b->root = insert(id, b, tamanho, 0);
                GravarDados(b->root, "arquivo.dat");
                bb->root = insert(rg,bb,tamanho, 1);
            }
        }

    }else{
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    while(busca != 0){
        printf("\n[0] Sair\n[1] Busca por RG\n[2] Busca por ID\n[3] Inserir novos dados (chave: RG)\n[4] Inserir novos dados (chave: ID)\n[5] Excluir dados (chave: RG)\n[6] Excluir dados (chave: ID)\n");
	    	scanf("%d", &busca);
        if(busca == 1 || busca == 2){
          if(busca == 1){
              printf("Informe o RG:\n");
              scanf("%d",&buscar);
              rs = search(buscar, bb->root);
	        }else{
                printf("Informe o ID:\n");
                scanf("%d",&buscar);
		        		rs = search(buscar, b->root);
        	}
          if(rs->found){
              printf("Informe o valor em bytes.\n");
		        	scanf("%ld",&bytes);
		        	fseek(arq, bytes, SEEK_SET);
              fscanf(arq,"%d %s %d %lf %d %s %c",&id, nome, &rg, &salario, &idade, departamento, &caracter);
	            printf("%d %s %d %.2lf %d %s %c\n",id, nome, rg, salario, idade, departamento, caracter);

            }else{
                printf("Chave não encontrada.\n");
                flag = 1;
            }

        }
        if(busca == 3 || busca == 4){
            printf("Informe os seguintes dados:\n");
            printf("ID: ");
            scanf("%d",&id);
            printf("Nome: ");
            scanf("%s",nome);
            printf("RG: ");
            scanf("%d",&rg);
            printf("Salario: ");
            scanf("%lf",&salario);
            printf("Idade: ");
            scanf("%d",&idade);
            printf("Departamento: ");
            scanf("%s",departamento);
            caracter = '#';

            tamanhoInsercao = ftell(arq);

            if(busca == 3){
                bb->root = insert(rg, bb, tamanhoInsercao, 1);
                GravarDados(bb->root, "arquivo.dat");
            }else{
                b->root = insert(id, b, tamanhoInsercao, 0);
                GravarDados(b->root, "arquivo.dat");
            }
        }

        if(busca == 5){
            printf("Informe o RG que deseja excluir.\n");
            scanf("%d",&rg);
            bb->root = delete(rg,bb);
            printf("Dados excluídos com sucesso!\n");
        }

        if(busca == 6){
            printf("Informe o ID que deseja excluir.\n");
            scanf("%d",&id);
            b->root = delete(id,b);
            printf("Dados excluídos com sucesso!\n");
        }
    }

    fclose(arq);
		fclose(file);
		fclose(filee);

	return 0;
}
