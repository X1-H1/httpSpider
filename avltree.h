#ifndef AVLTREE_H
#define AVLTREE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

//������(sxysxy)�Ĳ��� sxysxy.org/blogs/13
#include "basedef.h"
typedef ansiString elemType;
typedef struct _AVLNode
{
    struct _AVLNode *left, *right;
    elemType data;     //��ǰ�ڵ�����
    int height;   //���ߣ�����ƽ������
    int size;     //�Ե�ǰ�ڵ�Ϊ���������Ľڵ���
}AVLNode, *pNode, *AVLTree;
#define HEIGHT(x) ((x)?(x)->height:-1)
#define SIZE(x) ((x)?(x)->size:0)
void maintain(pNode k)
{       //ά�������ڵ����Ϣ
    if(!k)return;
    k -> height = max(HEIGHT(k->left), HEIGHT(k->right))+1;
    k -> size = 1 + SIZE(k -> left) + SIZE(k -> right);
}
/*             LL �����ӵ���߲�������ʱִ�� K1�����ӣ�K2���²����
                K0
               /                  K1
              K1    --K0 LL->    /  \ 
             /                  K2  K0
            K2
*/
pNode singleRotateLL(pNode k)
{
    pNode k1 = k -> left;
    k -> left = k1 -> right;
    k1 -> right = k;
    maintain(k);
    maintain(k1);
    return k1;
}

/*
                    RR ���Һ��ӵ��ұ߲�������
             K0                     K1
               \                   /  \
               K1     --K0 RR->   k0  k2
                \
                K2
*/
pNode singleRotateRR(pNode k)
{
    pNode k1 = k -> right;
    k -> right = k1 -> left;
    k1 -> left = k;
    maintain(k);
    maintain(k1);
    return k1;
}

/*
                  LR �����ӵ��ұ߲�������
            K0                K0                K1
           /                 /                 /  \
          K1    --K1 RR->   K1    --K0 LL->   K2  K0
           \               /
           K2             K2

*/
pNode doubleRotateLR(pNode k)
{
    k -> left = singleRotateRR(k -> left);
    return singleRotateLL(k);
}

/*
                   RL ���Һ��ӵ���߲�������
           K0               K0                   K1
             \               \                  /  \ 
             K1  --K1 LL->   K1     --K0 RR->  K0  K2
            /                 \
           K2                 K2
*/
pNode doubleRotateRL(pNode k)
{
    k -> right = singleRotateLL(k -> right);
    return singleRotateRR(k);
}

//����
pNode insert(AVLTree t, elemType x)
{ 
    if(!t)     //�½�
    {
        t = (AVLNode *)malloc(sizeof(AVLNode));;
        t -> data = x;
        t -> size = 1;
        t -> left = t -> right = NULL;
    }else
    {
        if(x < t -> data)   //Ӧ����������
        {
            t -> left = insert(t -> left, x);
            //�����ƽ��
            if(HEIGHT(t -> left) - HEIGHT(t -> right) == 2)
            {
                //��������ߵ����
                if(x < t -> left -> data)
                    t = singleRotateLL(t); //����
                else    
                    t = doubleRotateLR(t); //��˫��
            }
        }else //x >= t -> data ����������
        {
            t -> right = insert(t -> right, x);
            if(HEIGHT(t -> right) - HEIGHT(t -> left) == 2)
            {
                //�����ұߵ��ұ�
                if(x >= t -> right -> data)
                    t = singleRotateRR(t); //�ҵ��� 
                else
                    t = doubleRotateRL(t);
            }
        }
    }
    maintain(t);
    return t;
}

//ɾ��������ƽ��
pNode delBalance(AVLTree t)
{
    if(HEIGHT(t -> left) - HEIGHT(t -> right) == 2)
    {         //ɾ������߸���
        if(t -> left)
        {
            //���ӵ����ӱ����ӵ��Һ��Ӹߣ�����
            if(HEIGHT(t -> left -> left) >= HEIGHT(t -> left -> right))
                t = singleRotateLL(t);
            else
                t = doubleRotateLR(t);
        }
    }
    if(HEIGHT(t -> right) - HEIGHT(t -> left) == 2)
    {
        if(t -> right)
        {   
            //ͬ��
            if(HEIGHT(t -> right -> right) >= HEIGHT(t -> right -> left))
                t = singleRotateRR(t);
            else
                t = doubleRotateRL(t);
        }
    }
    maintain(t);
    return t;
}
//ɾ��
pNode delNode(AVLTree t,elemType x)
{
    if(!t)
        return t;
    if(x == t -> data)   //�ҵ�Ҫɾ���Ľڵ�
    {
        //û����������ֱ�����������Ӵ�������λ�á�
        if(t -> right == NULL)
        {
            pNode tmp = t;
            t = t -> left;
            free(tmp);
        }else
        {
            //�������������ҵ�����������С�Ľڵ㡣
            pNode tmp = t -> right;
            while(tmp -> left)
                tmp = tmp -> left;
            //�������滻��t��ֵ�����ǽ���tmp��t!!
            t -> data = tmp -> data;
            //��t����������ɾ��tmp
            t -> right = delNode(t -> right, t -> data);
        }
        maintain(t);
        return t;
    }else if(x < t -> data)
    {
        t -> left = delNode(t -> left, x);    //�����ɾ��
    }else
    {
        t -> right = delNode(t -> right, x);
    }

    //ά��ƽ���ά����Ϣ��
    if(t -> left)
        t -> left = delBalance(t -> left);
    if(t -> right)
        t -> right = delBalance(t -> right);
    if(t)
        t = delBalance(t);
    return t;
}

void destroy(AVLTree *t)
{
    AVLTree root = *t;
    if(!root)return;
    if(root -> left)
        destroy(&root -> left);
    if(root -> right)
        destroy(&root -> right);
    free(root);
    *t = NULL;
}
#endif