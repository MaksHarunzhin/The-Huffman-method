#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <iomanip>
#include <string>
#include <map>

#include "TNode.h"

using namespace std;

/// <summary>
/// Визуализация дерева Хаффмана.
/// Функция выводит на консоль сформированное дерево Хаффмана.
/// Вершины находиться слева -> справа от неё находяться вершины
/// </summary>
/// <param name="root"> Корень дерева Хаффмана </param>
/// <param name="space"> Количество отступов для вывода дерева </param>
/// <param name="level"> Уровень рассматриваемого узла в дереве </param>
void printTree(Node* root, int space, int level);

/// <summary>
/// Шаблонная функция построения дерева Хаффмана
/// </summary>
/// <typeparam name="T"> Тип контейнера </typeparam>
/// <param name="frequency"> Частота повторяемости символа </param>
/// <returns> Указатель на корень сформированного дерева </returns>
template <typename T>
Node* buildHuffmanTree(const T& frequency);

/// <summary>
/// Рекурсивная функция формирования кодов символов с учётом префиксного правила
/// </summary>
/// <param name="root"> Указатель на корень дерева Хаффмана </param>
/// <param name="str"> Строка, которую необходимо закодировать </param>
/// <param name="huffmanCodes"> Словарь в котором храняться символы (ключи) и их коды (значения) </param>
void buildCodes(Node* root, const string& str, map<char, string>& huffmanCodes);

/// <summary>
/// Функция сжатия текста (кодирования символов)
/// </summary>
/// <param name="inputFilePath"> Путь к файлу, в котором содержится текст для сжатия </param>
/// <param name="outputFilePath"> Путь к файлу, в который нужно записать закодированный текст и служебную информацию</param>
void compress(const string& inputFilePath, const string& outputFilePath);

/// <summary>
/// Функция декодирования закодированного текста
/// </summary>
/// <param name="root"> Указатель на дерево Хаффмана </param>
/// <param name="encodedString"> Строка с закодированным текстом </param>
/// <returns></returns>
string decode(Node* root, const string& encodedString);

/// <summary>
/// Функция распаковки закодированного текста из файла
/// </summary>
/// <param name="inputFilePath"> Путь к файлу с закодированным текстом </param>
/// <param name="outputFilePath"> Путь к файлу, в который нужно записать раскодированный текст </param>
void decompress(const string& inputFilePath, const string& outputFilePath);

/////////////////////////////////////////////////////////////////////

int main() {
    setlocale(LC_ALL, "RU");
    string inputFilePath = "input.txt";  // Путь к входному файлу
    string outputFilePath = "output.bin"; // Путь к выходному файлу

    compress(inputFilePath, outputFilePath);

    decompress("output.bin", "decoded.txt");

    cout << "Compression complete!" << endl;

    return 0;
}

/////////////////////////////////////////////////////////////////////


void printTree(Node* root, int space = 0, int level = 10) {
    // Базовый случай
    if (root == nullptr) {
        return;
    }

    // Увеличиваем расстояние между уровнями
    space += level;

    // Рекурсивно вызываем для правого поддерева
    printTree(root->right, space);

    // Вывод текущего узла после пробелов
    cout << endl;
    cout << setw(space) << root->ch << std::endl;

    // Рекурсивно вызываем для левого поддерева
    printTree(root->left, space);
}

// Компаратор для приоритетной очереди
struct Compare {
    bool operator()(Node* l, Node* r) {
        return l->freq > r->freq;
    }
};

template <typename T>
Node* buildHuffmanTree(const T& frequency) {
    // Приоритетная очередь
    priority_queue<Node*, vector<Node*>, Compare> minHeap;

    // Формирование очереди из словаря
    for (const auto& pair : frequency) {
        minHeap.push(new Node(pair.first, pair.second));
    }

    while (minHeap.size() > 1) {
        Node* left = minHeap.top();
        minHeap.pop();

        Node* right = minHeap.top();
        minHeap.pop();

        Node* newNode = new Node('0', left->freq + right->freq);
        newNode->left = left;
        newNode->right = right;

        minHeap.push(newNode);
    }

    printTree(minHeap.top());

    return minHeap.top();
}


void buildCodes(Node* root, const string& str, map<char, string>& huffmanCodes) {
    if (root == nullptr) {
        return;
    }

    if (root->left == nullptr && root->right == nullptr) {
        huffmanCodes[root->ch] = str;
        return;
    }

    buildCodes(root->left, str + "0", huffmanCodes);
    buildCodes(root->right, str + "1", huffmanCodes);
}

void compress(const string& inputFilePath, const string& outputFilePath) {
    // Подсчет частоты символов
    unordered_map<char, int> frequency;
    ifstream inputFile(inputFilePath);
    if (!inputFile.is_open()) {
        return;
    }

    char ch;

    while (inputFile.get(ch)) {
        if (ch == ' ') {
            frequency['_']++;
        }
        else {
            frequency[ch]++;
        }
    }

    // Формирование дерева Хаффмана
    Node* root = buildHuffmanTree(frequency);

    // Генерация кодов Хаффмана
    map<char, string> huffmanCodes;
    buildCodes(root, "", huffmanCodes);

    for (pair<char, string> p : huffmanCodes) {
        cout << p.first << " " << p.second << endl;
    }

    // Запись в выходной файл
    ofstream outputFile(outputFilePath, ios::binary);

    // Запись количества различных символов и их частот
    int uniqueCharsCount = static_cast<int>(frequency.size());
    outputFile << uniqueCharsCount << "\n";

    for (const auto& pair : frequency) {
        outputFile << pair.first << " " << pair.second << "\n";
    }

    // Сжатие текста
    string encodedString;
    inputFile.clear();
    inputFile.seekg(0);

    while (inputFile.get(ch)) {
        // Заменяем пробле на символ "_"
        if (ch == ' ') {
            ch = '_';
        }
        encodedString += huffmanCodes[ch];
    }

    // Количество полезных бит в последнем байте
    int padding = 8 - (encodedString.size() % 8);

    // Добавление паддинга к строке
    for (int i = 0; i < padding && padding != 8; ++i) {
        encodedString += '0';
    }

    outputFile << 8 - padding << "\n";

    // Запись сжатого текста побайтово
    for (size_t i = 0; i <= encodedString.size(); i += 8) {
        bitset<8> byte(encodedString.substr(i, 8));
        unsigned long n = byte.to_ulong();
        outputFile << reinterpret_cast<const char*>(&n);
    }

    inputFile.close();
    outputFile.close();
}


// Функция для декодирования закодированного текста
string decode(Node* root, const string& encodedString) {
    string decodedString;
    Node* currentNode = root;

    for (char bit : encodedString) {
        if (bit == '0') {
            currentNode = currentNode->left;
        }
        else {
            currentNode = currentNode->right;
        }

        // Если достигли листа дерева Хаффмана (крайнего элемента)
        if (!currentNode->left && !currentNode->right) {
            if (currentNode->ch == '_') {
                decodedString += " ";
            }
            else {
                decodedString += currentNode->ch;
            }
            currentNode = root; // Вернуться к корню
        }
    }

    return decodedString;
}

// Функция для декомпрессии файла
void decompress(const string& inputFilePath, const string& outputFilePath) {
    ifstream inputFile(inputFilePath, ios::binary);

    // Чтение количества уникальных символов
    int uniqueCharsCount = 0;
    inputFile >> uniqueCharsCount;

    // Чтение символов и их частот
    vector<pair<char, int>> frequency;
    for (int i = 0; i < uniqueCharsCount; ++i) {
        char ch;
        int freq;
        inputFile >> ch;
        inputFile >> freq;
        pair<char, int> p = { ch, freq };
        frequency.push_back(p);
    }
    for (auto i : frequency) {
        cout << i.first << " " << i.second << endl;
    }
    // Восстановление дерева Хаффмана
    Node* root = buildHuffmanTree(frequency);

    // Чтение количества полезных бит в последнем байте
    int padding = 0;
    inputFile >> padding;

    // Чтение закодированных данных
    string encodedString;
    char byteString;
    while (inputFile.get(reinterpret_cast<char&>(byteString))) {
        if (byteString == '\0' || byteString == '\n') {
            continue;
        }

        bitset<8> byte(byteString);
        encodedString += byte.to_string();
    }

    // Удаление паддинга
    encodedString = encodedString.substr(0, encodedString.size() - padding);

    // Декодирование строки
    string decodedString = decode(root, encodedString);

    // Запись декодированного текста в выходной файл
    ofstream outputFile(outputFilePath);
    outputFile << decodedString;

    // Освобождение памяти
    delete root;

    inputFile.close();
    outputFile.close();
}