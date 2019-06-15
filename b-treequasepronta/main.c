#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define TRUE 1
#define FALSE 0
#define EMPTY 0

#define NODE_ORDER		51 /*The degree of the tree.*/
#define NODE_POINTERS	(NODE_ORDER*2)
#define NODE_KEYS		NODE_POINTERS-1

typedef unsigned char bool;

int inseridosMat = 0;
int inseridosRg = 0;
int tamTotal;

//gravar node
typedef struct tree_node {
	int key_array[NODE_KEYS];
	long int child_array[NODE_POINTERS]; //
	unsigned int key_index;
    long int tamanho[NODE_KEYS];
	bool leaf;
} node_t;

typedef struct {
	long int node_pointer;//
	int key;
	bool found;
	unsigned int depth;
} result_t;

//cabeçalho
typedef struct {
	node_t *root; //
	unsigned short order;
	bool lock;
} btree_t;

static int BTreeGetLeftMax(node_t *T);
static int BTreeGetRightMin(node_t *T);

static node_t *create_node(int id){
	FILE *file;
	FILE *filee;
	file = fopen("arquivoMat.dat","ab");
	filee = fopen("arquivoRg.dat","ab");
	int i;
	long int tamPagina, tamPaginaa;
	node_t *new_node = (node_t *)malloc(sizeof(node_t));

	if(!new_node){
		printf("Out of memory");
		exit(0);
	}

	// Set Keys
	for(i = 0;i < NODE_KEYS; i++){
		new_node->key_array[i] = 0;
	}

	// Set ptr
	for(i = 0;i < NODE_POINTERS; i++){
		new_node->child_array[i] = -1;
	}

	new_node->key_index = EMPTY;
	new_node->leaf = TRUE;

	if(id == 0){ //arquivo matricula
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

	//ret->node_pointer = NULL; //acho q nao vai precisar entao
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
	//print_node(node);
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
		result->node_pointer = node->tamanho[i]; //recebe node->tamanho[i]
		result->found = FALSE;
    //printf("A matricula %d não pode ser encontrada.\n", key);
		return result;
	}else{
		result_t *result = get_resultset();
		return search(key, node->child_array[i]);
	}
}

static void split_child(node_t *parent_node, int i, long int child_array[], int id, FILE *file){
	int j,k,flag = 1, key[NODE_KEYS];
	long int c, tam[NODE_KEYS], cc[NODE_POINTERS];
	unsigned int key_i;

	node_t *new_node = create_node(id);
	for(k=0;k<NODE_POINTERS;k++){
		if(child_array[k] != -1){
			flag = 0;
		}
	}

	if(flag == 0){//nao é folha
		new_node->leaf = 0;
	}else{//é folha
		new_node->leaf = 1;
	}

	new_node->key_index = NODE_ORDER-1;

	for(j = 0;j < NODE_ORDER-1;j++){
		fseek(file, child_array, SEEK_SET);
		fscanf(file,"%ld %d %ld %ld, %i",&c, &key, &tam, &cc, &key_i);
		new_node->key_array[j] = key[NODE_ORDER+j];
    	new_node->tamanho[j] = tam[NODE_ORDER+j];
	}

	if(flag == 0){
		for(j = 0; j < NODE_ORDER; j++){
			new_node->child_array[j] = cc[NODE_ORDER+j];
		}
	}
	key_i = NODE_ORDER-1;


	for(j = parent_node->key_index;j>=i;j--){
		parent_node->child_array[j+1] = parent_node->child_array[j];
	}

	parent_node->child_array[i] = new_node;

	for(j = parent_node->key_index;j>=i;j--){
		parent_node->key_array[j] = parent_node->key_array[j-1];
    	parent_node->tamanho[j]=parent_node->tamanho[j-1];
	}

	parent_node->key_array[i-1] = key[NODE_ORDER-1];
    parent_node->tamanho[i-1] = tam[NODE_ORDER-1];

	parent_node->key_index++;
}

static void insert_nonfull(node_t *n, int key, long int tamanho, int id, FILE *file){
	unsigned int key_i;
	int i = n->key_index;

	if(n->leaf){
        //printf("%d %ld\n",key,tamanho);
		// Shift until we fit
		while(i>=1 && key<n->key_array[i-1]){
			n->key_array[i] = n->key_array[i-1];
            n->tamanho[i]=n->tamanho[i-1];
			i--;
		}

		n->key_array[i] = key;
        //printf("%d\n", n->key_index);

        n->tamanho[i] = tamanho;
        //printf("%d %d %ld\n",n->key_array[i],n->key_index,n->tamanho[i]);
        //printf ("tamanho em bytes inseridos [pos inseridos: %d, index da página: %d, pos i: %d]: %ld; \n", inseridosMat, n->key_index,i,tamanho);
        n->key_index++;
        inseridosMat++;

	}else{
		// Find the position i to insert.
		while(i>=1 && key<n->key_array[i-1]){
			i--;
		}
		fseek(file, n->child_array[i], SEEK_SET);
		fscanf(file,"%i",&key_i);
		if(n->key_index == NODE_KEYS){
			split_child(n, i+1, n->child_array[i], id, file);
			if(key > n->key_array[i]){
				i++;
			}
		}
		//Recursive insert.
		insert_nonfull(n->child_array[i], key, tamanho, id, file);
	}
}

node_t *insert(int key, btree_t *b, long int tamanho, int id, FILE *file){
	if(!b->lock){
		node_t *root = b->root;
		if(root->key_index == NODE_KEYS){ //If node root is full,split it.
			node_t *newNode = create_node(id);
			b->root = newNode; //Set the new node to T's Root.
			newNode->leaf = 0;
			newNode->key_index = 0;
      //newNode->tamanho = tamanho;
			newNode->child_array[0] = root;
			split_child(newNode, 1, root, id, file);//Root is 1th child of newNode.
			insert_nonfull(newNode, key, tamanho, id, file); //Insert X into non-full node.
		}else{ //If not full,just insert X in T.
			insert_nonfull(b->root, key, tamanho, id, file);
		}
	}else{
		printf("Tree is locked\n");
	}

	return b->root;
}

static void merge_children(node_t *root, int index, long int child1, long int child2, FILE *file){
	int key[NODE_KEYS], key2[NODE_KEYS], k, flag = 1;
	long int tam, tam2, cc[NODE_POINTERS];
	unsigned int key_i;
	fseek(file, child1, SEEK_SET);
	fscanf(file,"%d %ld %d",&key, &tam, &key_i);
	fseek(file, child2, SEEK_SET);
	fscanf(file,"%d %ld %ld",&key2, &tam2, &cc);
	key_i = NODE_KEYS;
	int i;

	for(i=NODE_ORDER;i<NODE_KEYS;i++){
		key[i] = key2[i-NODE_ORDER];
    	tam[i] = tam2[i-NODE_ORDER];
  	}
	key[NODE_ORDER-1] = root->key_array[index];
  	tam[NODE_ORDER-1] = root->tamanho[index];

	for(k=0;k<NODE_POINTERS;k++){
		if(child2[k] != -1){
			flag = 0;
		}
	}

	if(flag == 1){//se é folha
		for(i=NODE_ORDER;i<NODE_POINTERS;i++)
			child1->child_array[i] = cc[i-NODE_ORDER];
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
	if(0 == leftPtr->leaf)
		for(i=curPtr->key_index;i>0;i--)
			curPtr->child_array[i] = curPtr->child_array[i-1];
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
	//Is root is a leaf node ,just delete it.
	if(1 == root->leaf){
		i=0;
		while( (i<root->key_index) && (X>root->key_array[i])) //Find the index of X.
			i++;
		//If exists or not.
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
	else{  //X is in a internal node.
		i = 0;
		node_t *prePtr = NULL, *nexPtr = NULL;
		//Find the index;
		while( (i<root->key_index) && (X>root->key_array[i]) )
			i++;
		if( (i<root->key_index) && (X == root->key_array[i]) ){ //Find it in this level.
			prePtr = root->child_array[i];
			nexPtr = root->child_array[i+1];
			/*If prePtr at least have d keys,replace X by X's precursor in
			 *prePtr*/
			if(prePtr->key_index > NODE_ORDER-1){
				int aPrecursor = BTreeGetLeftMax(prePtr);
				root->key_array[i] = aPrecursor;
                root->tamanho[i] = aPrecursor;
				//Recursively delete aPrecursor in prePtr.
				BTreeDeleteNoNone(aPrecursor,prePtr);
			}
			else
			if(nexPtr->key_index > NODE_ORDER-1){
			/*If nexPtr at least have d keys,replace X by X's successor in
			 * nexPtr*/
				int aSuccessor = BTreeGetRightMin(nexPtr);
				root->key_array[i] = aSuccessor;
                root->tamanho[i] = aSuccessor;
				BTreeDeleteNoNone(aSuccessor,nexPtr);
			}
			else{
			/*If both of root's two child have d-1 keys,then merge root->K[i]
			 * and prePtr nexPtr. Recursively delete X in the prePtr.*/
				merge_children(root,i,prePtr,nexPtr);
				BTreeDeleteNoNone(X,prePtr);
			}
		}
		else{ //Not find in this level,delete it in the next level.
			prePtr = root->child_array[i];
			node_t *leftBro = NULL;
			if(i<root->key_index)
				nexPtr = root->child_array[i+1];
			if(i>0)
				leftBro = root->child_array[i-1];

			if(NODE_ORDER-1 == prePtr->key_index){
			//If left-neighbor have at least d-1 keys,borrow.
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
    int matricula, rg, idade, busca = -1, i, buscar;
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
	file = fopen("arquivoMat.dat","ab");

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

            i = fscanf(arq,"%d %s %d %lf %d %s %c",&matricula, nome, &rg, &salario, &idade, departamento, &caracter);
			/*if(paiJaFoi == 0){
				paiJafoi = 1;
				tamPai = ftell(arq);
			}*/
            if(i == 7){

                b->root = insert(matricula, b, tamanho, 0);
                //GravarDados(b->root, "arquivoMat.dat");
                bb->root = insert(rg,bb,tamanho, 1);
                //GravarDados(bb->root, "arquivoRg.dat");
            }
        }

    }else{
        printf("Erro ao abrir o arquivo.\n");
        return 1;
    }

    while(busca != 0){
        printf("\n[0] Sair\n[1] Busca por RG\n[2] Busca por matrícula\n[3] Inserir novos dados (chave: RG)\n[4] Inserir novos dados (chave: matrícula)\n[5] Excluir dados (chave: RG)\n[6] Excluir dados (chave: matrícula)\n");
	    scanf("%d", &busca);
        if(busca == 1 || busca == 2){
            if(busca == 1){
                printf("Informe o RG:\n");
                scanf("%d",&buscar);
                rs = search(buscar, bb->root);

	        }else{
                printf("Informe a matrícula:\n");
                scanf("%d",&buscar);
		        rs = search(buscar, b->root);
            }
            if(rs->found){
                printf("Informe o valor em bytes.\n");
		        scanf("%ld",&bytes);
		        fseek(arq, bytes, SEEK_SET);
                fscanf(arq,"%d %s %d %lf %d %s %c",&matricula, nome, &rg, &salario, &idade, departamento, &caracter);
	            printf("%d %s %d %.2lf %d %s %c\n",matricula, nome, rg, salario, idade, departamento, caracter);

            }else{
                printf("Chave não encontrada.\n");
                flag = 1;
            }

        }
        if(busca == 3 || busca == 4){
            printf("Informe os seguintes dados:\n");
            printf("Matrícula: ");
            scanf("%d",&matricula);
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
				tamanhoInsercao = ftell(arq);
                bb->root = insert(rg, bb, tamanhoInsercao, 1);
                //GravarDados(bb->root, "arquivoRg.dat");
            }else{
				tamanhoInsercao = ftell(arq);
                b->root = insert(matricula, b, tamanhoInsercao, 0);
                //GravarDados(b->root, "arquivoMat.dat");
            }
        }

        if(busca == 5){
            printf("Informe o RG que deseja excluir.\n");
            scanf("%d",&rg);
            bb->root = delete(rg,bb);
            printf("Dados excluídos com sucesso!\n");
        }

        if(busca == 6){
            printf("Informe a matrícula que deseja excluir.\n");
            scanf("%d",&matricula);
            b->root = delete(matricula,b);
            printf("Dados excluídos com sucesso!\n");
        }
    }

    fclose(arq);

	return 0;
}
