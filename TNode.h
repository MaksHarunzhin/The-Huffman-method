#pragma once


// ��������� ��� ���� ������ ��������
struct Node {
    char ch;
    int freq;
    Node* left, * right;

    Node(char character, int frequency) : ch(character), freq(frequency), left(nullptr), right(nullptr) {}
};