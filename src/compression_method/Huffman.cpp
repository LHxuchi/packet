#include "../../include/compression_method/Huffman.h"
std::unique_ptr<data_packet::Huffman::Huffman_tn> data_packet::Huffman::Create_Huffman_Tree()
{
    std::unique_ptr<Huffman_tn> head;

    std::priority_queue<std::unique_ptr<Huffman_tn>, std::vector<std::unique_ptr<Huffman_tn>>, cmp> q;
    for (uint32_t i = 0; i < 256; i++)
    {
        if (times[i] <= 0)
            continue;
        std::unique_ptr<Huffman_tn> tmp(new Huffman_tn(times[i], i));
        q.push(std::move(tmp));
    }
    while (q.size() > 1)
    {
        std::unique_ptr<Huffman_tn> node_a = std::move(const_cast<std::unique_ptr<Huffman_tn>&>(q.top()));
        q.pop();
        std::unique_ptr<Huffman_tn> node_b = std::move(const_cast<std::unique_ptr<Huffman_tn>&>(q.top()));
        q.pop();

        head = std::make_unique<Huffman_tn>(node_a->weight + node_b->weight);
        if (node_a->weight > node_b->weight)//保持严格序，使得每次能建出相同的树，左小右大
        {
            swap(node_a, node_b);
        }
        head->lson = std::move(node_a);
        head->rson = std::move(node_b);
        q.push(std::move(head));
    }
    if (!q.empty())
    {
        head = std::move(const_cast<std::unique_ptr<Huffman_tn>&>(q.top()));
        q.pop();
    }
    if (head!=nullptr&&head->word != -1)
    {
        auto tmp = std::move(head);
        head = std::make_unique<Huffman_tn>(tmp->weight);
        head->lson = std::move(tmp);
    }
    return head;
}


void data_packet::Huffman::encoding_dfs(Huffman_tn* node, std::string prefix)
{
    if (node == nullptr)
        return;
    if (node->word != -1)
    {
        Huffman_coding[(uint8_t)node->word] = prefix;
        return;
    }
    encoding_dfs(node->lson.get(), prefix + "0");//左0右1
    encoding_dfs(node->rson.get(), prefix + "1");
}