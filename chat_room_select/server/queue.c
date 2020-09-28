#include"config.h"

void init(pQueue pq)
{
	pNode pn = (pNode)malloc(sizeof(Node));
	if (!pn)
		perror("malloc failed\n");

	pn->data = 0;	//空结点的data表示队列长度
	pn->next = NULL;
	pq->back = pq->front = pn;
}

void destroy(pQueue pq)
{
	while (!is_empty(pq))
	{
		pop(pq);
	}

	free(pq->front);
	pq->back = pq->front = NULL;
}

int is_empty(pQueue pq)
{
	return pq->front == pq->back;
}

//int size(pQueue pq)
//{
//	int n = 0;
//	pNode p = pq->front->next;
//
//	while (p)
//	{
//		p = p->next;
//		++n;
//	}
//
//	return n;
//}

int size(pQueue pq)
{
	/*空结点的data表示队列长度*/
	return pq->front->data;
}

int back(pQueue pq)
{
	return pq->back->data;
}

/*获得队列第一个元素的值*/
int front(pQueue pq)
{
	return pq->front->next->data;
}

void push(pQueue pq, int *e)
{
	pNode pn = (pNode)malloc(sizeof(Node));
	if (!pn)
		perror("malloc failed\n");

	pn->data = *e;
	
	pq->back->next = pn;
	pq->back = pn;

	/*队列长度+1*/
	++pq->front->data;

	return;
}

void pop(pQueue pq, int *e)
{
	/*pq->front一直指向空结点,pq->front->next才是第一个结点*/

	if (is_empty(pq))
		return;

	pNode pn = pq->front->next;
	pq->front->next = pn->next;

	*e = pn->data;
	free(pn);

	/*队列长度-1*/
	--pq->front->data;

	return;
}