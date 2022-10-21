#include <iostream>
#include <thread>
#include <pthread.h>
#include <ctime>
#include <cstdlib>
#include <string>
#include <semaphore.h>
#define N 10  // Numero de threads
#define T 100 // para el semaforo

using namespace std;
pthread_t tid[N];			 // Threads
int mejorglobal = INT32_MAX; // Peso global
string caminoglobal;		 // Camino del mejor peso
pthread_mutex_t lock;		 // Inicializar variable de mutex
int num_sem = 0;
sem_t S[T];

// Nodo con su respectiva arista
struct nodo
{
	char letra;
	struct nodo *sgte;
	struct arista *ady;
};

typedef struct nodo *Tnodo;
Tnodo puntero;

// estructura de arista con su peso y siguiente arista en el caso de tener
struct arista
{
	struct nodo *destino;
	struct arista *sgte;
	int peso;
	int id; // Numero de bloqueo de semaforo
};

typedef struct arista *Tarista;

// Insertar un nuevo nodo sin aristas conectadas
void insertar_nodo(char nodo)
{
	Tnodo t, nuevo = new struct nodo;
	nuevo->letra = nodo;
	nuevo->sgte = NULL;
	nuevo->ady = NULL;
	if (puntero == NULL)
	{
		puntero = nuevo;
	}
	else
	{
		t = puntero;
		while (t->sgte != NULL)
		{
			t = t->sgte;
		}
		t->sgte = nuevo;
	}
}

// Llamada de la funcion insertar arista que le pasa con los datos
void agregar_arista(Tnodo &aux, Tnodo &aux2, Tarista &nuevo)
{
	Tarista q;
	if (aux->ady == NULL)
	{
		aux->ady = nuevo;
		nuevo->destino = aux2;
	}
	else
	{
		q = aux->ady;
		while (q->sgte != NULL)
		{
			q = q->sgte;
		}
		nuevo->destino = aux2;
		q->sgte = nuevo;
	}
}

// Insertar arista
void insertar_arista(char ini, char fin, int pesoo)
{
	Tarista nuevo = new struct arista;
	// Se agrega el peso y el id
	nuevo->peso = pesoo;
	nuevo->id = num_sem;
	// Se crea un random entre el 1 y el 5 el cual dira cuantos threads simultaneos pueden estar en la la arista x como SC
	int num = (rand() % 5) + 1;
	sem_init(&S[num_sem], 0, num);
	num_sem++;
	// cout << nuevo->sem << endl;
	Tnodo aux, aux2;
	if (puntero == NULL)
	{
		cout << "No hay Grafo" << endl;
		return;
	}
	nuevo->sgte = NULL;

	aux = puntero;
	aux2 = puntero;
	while (aux2 != NULL)
	{
		if (aux2->letra == fin)
		{
			break;
		}
		aux2 = aux2->sgte;
	}
	while (aux != NULL)
	{
		if (aux->letra == ini)
		{
			agregar_arista(aux, aux2, nuevo);
			return;
		}
		aux = aux->sgte;
	}
}

// Mostrar grafo
void mostrar_grafo()
{
	Tnodo ptr;
	Tarista ar;
	ptr = puntero;
	cout << "Nodo : Adyacencia" << endl;
	while (ptr != NULL)
	{
		cout << "   " << ptr->letra << "|";
		if (ptr->ady != NULL)
		{
			ar = ptr->ady;
			while (ar != NULL)
			{
				cout << " " << ar->destino->letra << "-" << ar->peso;
				ar = ar->sgte;
			}
		}
		ptr = ptr->sgte;
		cout << endl;
	}
}

// Inicializacion de nodos y aristas para creacion de grafo
void inicializar_grafo()
{
	// Ideal que el nodo A sea el inicial siempre
	insertar_nodo('A');
	insertar_nodo('B');
	insertar_nodo('C');
	insertar_nodo('D');
	insertar_nodo('E');
	insertar_nodo('F');
	insertar_nodo('G');
	insertar_nodo('H');
	insertar_nodo('I');
	// El nodo K siemore debe ser el nodo final
	insertar_nodo('K');
	// para insertar arista se le pasa el nodo inicio, nodo final, peso
	insertar_arista('A', 'B', 7);
	insertar_arista('A', 'C', 5);
	insertar_arista('A', 'D', 1);
	insertar_arista('B', 'H', 3);
	insertar_arista('C', 'H', 11);
	insertar_arista('D', 'H', 10);
	insertar_arista('H', 'E', 2);
	insertar_arista('H', 'F', 6);
	insertar_arista('H', 'G', 7);
	insertar_arista('H', 'I', 1);
	insertar_arista('E', 'K', 9);
	insertar_arista('F', 'K', 6);
	insertar_arista('G', 'K', 9);
	insertar_arista('I', 'K', 6);
}

// Esta funcion ve aleatoriamiente por que arista pasará para ir a otro nodo
Tnodo cambionodo(Tnodo actual, int *peso)
{
	Tarista aris = actual->ady;

	int conteo = 1;
	// Siempre empieza por el nodo A entonces no se necesita recorrer nodos para comenzar
	while (aris->sgte != NULL)
	{
		conteo++;
		aris = aris->sgte;
	}
	aris = actual->ady;
	// Random para ver por que camino se ira
	int num = (rand() % conteo);
	// out<<"conteo "<<num<<endl;
	for (int i = 0; i < num; i++)
	{
		aris = aris->sgte;
	}
	// Ya encontrado el camino a tomar se bloquea con un semaforo hasta que termine
	sem_wait(&S[aris->id]);
	int a = aris->peso;
	*peso = *peso + a;
	actual = aris->destino;
	sem_post(&S[aris->id]);
	return actual;
}

int VerificarPeso(string camino, int peso, int mejorpeso)
{
	// Se verifica si el peso sacado es mejor que el que tiene el thread
	if (peso < mejorpeso)
	{
		mejorpeso = peso;
		// Ahora se verifica si es mejor que el global, se bloquea el paso ya que en caso de serlo hay una modificación a un SC
		pthread_mutex_lock(&lock);
		if (mejorpeso < mejorglobal)
		{
			mejorglobal = mejorpeso;
			caminoglobal = camino;
			cout << "Nuevo mejor peso global: " << mejorglobal << endl;
		}
		pthread_mutex_unlock(&lock);
	}
	return mejorpeso;
}

void *thread_function(void *arg)
{
	// cout<<"Soy un thread\n";
	// mejor peso guarda el mejor peso sacado en el thread, peso total es el peso actual del camino
	int mejorpeso = INT32_MAX, pesototal;
	// Los mismo el mejorcamino es el camino del mejor peso sacado y el camino del peso actual
	string mejorcamino, camino;
	Tnodo nodo;
	// Cuantas veces el thread hara un recorrido por el grafo
	int rep = 5;
	// Pasar nuevamente por otro camino rep veces
	for (int i = 0; i < rep; i++)
	{
		pesototal = 0;
		nodo = puntero;
		camino = nodo->letra;

		while (1)
		{
			nodo = cambionodo(nodo, &pesototal);
			// cout<<pesototal<<endl;
			camino = camino + " " + nodo->letra + " ";
			if (nodo->letra == 'K')
				break; // Ultimo nodo rompe
		}
		// cout<<"El camino que tomo es "<< camino << " con un peso de "<< pesototal<<endl;
		mejorpeso = VerificarPeso(camino, pesototal, mejorpeso);
	}
	// cout << "Mejor peso de un thread fue de " << mejorpeso << endl;
}

int main()
{
	srand((unsigned)time(NULL));
	inicializar_grafo();
	mostrar_grafo();
	pthread_mutex_init(&lock, NULL);

	/*for (int i = 0; i < T; i++)
	{
		sem_init(&S[i], 0, 1);
	}*/

	for (int i = 0; i < N; i++)
	{
		pthread_create(&(tid[i]), NULL, &thread_function, NULL);
	}
	for (int i = 0; i < N; i++)
	{
		pthread_join(tid[i], NULL);
	}
	pthread_mutex_destroy(&lock);
	cout << "El mejor camino encontrado es " << caminoglobal << " con un peso de " << mejorglobal << endl;

	sem_destroy(S);

	return 0;
}