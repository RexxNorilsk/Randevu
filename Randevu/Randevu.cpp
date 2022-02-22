#include <iostream>
#include "mpi.h"
#include <Windows.h>
#include <fstream>
#include <cstdlib>
#include <time.h>


#define PERCENT_NOREQUEST 50

using namespace std;


int randomRange(int max, int min = 1) {
	return min + rand() % (max - min);
}

struct Queue
{
	int* clients;
	int length = 0;
	int pointer = 0;
};


void rendez_vous(int id, int rank) {
	if (rank == 0) {
		int buf;
		MPI_Send(&buf, 1, MPI_INT, id, 0, MPI_COMM_WORLD);
		cout << "[Server] Client " << id << "" << " I'm ready to receive your request" << endl;
		MPI_Recv(&buf, 1, MPI_INT, id, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		Sleep(randomRange(1500, 500));
		cout << "[Server] Message by client " << id << ": " << buf << endl;
	}
	else {
		int dat = randomRange(32, 1);
		MPI_Send(&dat, 1, MPI_INT, id, 0, MPI_COMM_WORLD);
	}
}

int main(int argv, char** argc)
{
	int size, rank;
	if (MPI_Init(&argv, &argc) != MPI_SUCCESS)//Проверка на инициализацию
		return 1;
	if (MPI_Comm_size(MPI_COMM_WORLD, &size) != MPI_SUCCESS)//Получение размера коммуникатора
		return 2;
	if (MPI_Comm_rank(MPI_COMM_WORLD, &rank) != MPI_SUCCESS)//Получение текущего ранга 
		return 3;
	if (size < 2)return 4;

	
	MPI_Status status;
	srand(rank*time(0)+ time(0));
	for (int t = 0; t < 3; t++) {
		//Сервер
		if (rank == 0) {
			cout << "[Server] ====== Day: " << t << " ====== " << endl;
			//Создаём очередь
			Queue queue;
			queue.length = size - 1;
			queue.clients = (int*)malloc(queue.length);
			memset(queue.clients, -1, queue.length);
			
			//Опрос Клиентов и занесение в очередь
			for (int i = 1; i < size;i++) {
				int buf = 1;
				MPI_Send(&buf, 1, MPI_INT, i, 1, MPI_COMM_WORLD);
				cout << "[Server] Client " << i << " you will be sending a request today?" << endl;
				MPI_Recv(&buf, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
				cout << "[Client " << i << "] " << (buf >= PERCENT_NOREQUEST ? "Yes" : "No") << endl;
				if (buf >= PERCENT_NOREQUEST) {
					queue.clients[queue.pointer++] = i;
					cout << "[Server] Client " << i << " added to the queue by number " << queue.pointer-1 << endl;
				}
				Sleep(randomRange(150, 100));
			}

			//Работа с очередью
			for (int i = 0; i < queue.pointer; i++) {
				rendez_vous(queue.clients[i], rank);
			}


		}
		//Клиенты
		else {
			int buf;
			MPI_Recv(&buf, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			buf = randomRange(100, 0);
			MPI_Send(&buf, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);

			if (buf >= PERCENT_NOREQUEST) {
				MPI_Recv(&buf, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				rendez_vous(0, rank);
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);
		Sleep(randomRange(3000, 1000));
	}
	
	
	MPI_Finalize();
}