#include <bits/stdc++.h>

namespace DB {
// Declaration
template <typename K, typename... V>
class BTreeNode;

template <typename K, typename... V>
class BTree;

// implementation
template <typename K, typename... V>
class BTreeNode {
   protected:
    typedef std::tuple<K, V...> Record;

    std::vector<K> keys;
    int degree;
    std::vector<BTreeNode<K, V...> *> children;

    friend class BTree<K, V...>;

   public:
    BTreeNode(int degree) : degree(degree), {
        this->keys.clear();
        this->children = std::vector<BTreeNode<K, V...> *>({nullptr});
    }

    BTreeNode(int degree, std::vector<K> keys,
              std::vector<BTreeNode<K, V...> *> children)
        : degree(degree), keys(keys), children(children) {}

    virtual ~BTreeNode() {
        for (auto child : this->children) {
            delete child;
        }
    }

    virtual bool insert(K key, Record rec) = 0;
    virtual void remove(K key) = 0;
    virtual Record search(K key) = 0;

    virtual K splitSelf(BTreeNode<K, V...> **left,
                        BTreeNode<K, V...> **right) = 0;
};

template <typename K, typename... V>
class InterNode : public BTreeNode<K, V...> {
   public:
    typedef std::tuple<K, V...> Record;

    InterNode(int degree) : BTreeNode<K, V...>(degree) {}
    InterNode(int degree, std::vector<K> keys,
              std::vector<BTreeNode<K, V...> *> children)
        : BTreeNode<K, V...>(degree, children) {}

    // IMplement functions
    bool insert(int key, Record rec) override {
        int idx = std::upper_bound(this->keys.begin(), this->keys.end(), key) -
                  this->keys.begin();

        BTreeNode<K, V...> *child = this->children[idx];
        bool isInserted = child->insert(key, rec);

        // std::cout << "isInserted: " << isInserted << std::endl;

        if (!isInserted) {
            BTreeNode<K, V...> *left, *right;
            K pushedKey = child->splitSelf(&left, &right);

            this->keys.insert(this->keys.begin() + idx, pushedKey);
            this->children[idx] = left;
            this->children.insert(this->children.begin() + idx + 1, right);
        }

        return this->keys.size() <= this->degree;
    }

    void remove(int key) override {
        std::cout << "Remove key " << key << std::endl;
    }
    Record search(int key) override {
        std::cout << "Search key " << key << std::endl;
        return Record();
    }

    K splitSelf(BTreeNode<K, V...> **left,
                BTreeNode<K, V...> **right) override {
        int mid = this->keys.size() / 2;
        std::vector<K> leftKeys(this->keys.begin(), this->keys.begin() + mid);
        std::vector<K> rightKeys(this->keys.begin() + mid + 1,
                                 this->keys.end());

        std::vector<BTreeNode<K, V...> *> leftChildren(
            this->children.begin(), this->children.begin() + mid + 1);
        std::vector<BTreeNode<K, V...> *> rightChildren(
            this->children.begin() + mid + 1, this->children.end());

        *left = new InterNode<K, V...>(this->degree, leftKeys, leftChildren);
        *right = new InterNode<K, V...>(this->degree, rightKeys, rightChildren);

        return rightKeys[0];
    }
};

template <typename K, typename... V>
class LeafNode : public BTreeNode<K, V...> {
   private:
    LeafNode<K, V...> *next;
    LeafNode<K, V...> *prev;

   public:
    typedef std::tuple<K, V...> Record;
    LeafNode(int degree)
        : BTreeNode<K, V...>(degree), next(nullptr), prev(nullptr) {}
    LeafNode(int degree, std::vector<K> keys,
             std::vector<BTreeNode<K, V...> *> children)
        : BTreeNode<K, V...>(degree, keys, children),
          next(nullptr),
          prev(nullptr) {}

    // Implement functions
    bool insert(K key, Record rec) override {
        int idx = std::upper_bound(this->keys.begin(), this->keys.end(), key) -
                  this->keys.begin();
        this->keys.insert(this->keys.begin() + idx, key);
        // std::cout << "size: " << this->keys.size()
        //           << " degree: " << this->degree << std::endl;
        return this->keys.size() <= this->degree;
    }

    void remove(int key) override {
        std::cout << "Remove key " << key << std::endl;
    }
    Record search(int key) override {
        std::cout << "Search key " << key << std::endl;
        return Record();
    }

    K splitSelf(BTreeNode<K, V...> **left,
                BTreeNode<K, V...> **right) override {
        int mid = this->keys.size() / 2;
        std::vector<K> leftKeys(this->keys.begin(), this->keys.begin() + mid);
        std::vector<K> rightKeys(this->keys.begin() + mid, this->keys.end());

        std::vector<BTreeNode<K, V...> *> leftChildren({nullptr});
        std::vector<BTreeNode<K, V...> *> rightChildren({nullptr});

        LeafNode<K, V...> *leftLeaf =
            new LeafNode<K, V...>(this->degree, leftKeys, leftChildren);
        LeafNode<K, V...> *rightLeaf =
            new LeafNode<K, V...>(this->degree, rightKeys, rightChildren);

        leftLeaf->prev = this->prev;
        leftLeaf->next = rightLeaf;

        rightLeaf->prev = leftLeaf;
        rightLeaf->next = this->next;

        if (this->prev != nullptr) {
            this->prev->next = leftLeaf;
        }

        if (this->next != nullptr) {
            this->next->prev = rightLeaf;
        }

        *left = leftLeaf;
        *right = rightLeaf;

        return rightKeys[0];
    }
};

template <typename K, typename... V>
class BTree {
   private:
    BTreeNode<K, V...> *root;
    int degree;

   public:
    BTree<K, V...>(int degree) : degree(degree) {
        this->root = new LeafNode<K, V...>(degree);
    }

    void insert(int key, typename BTreeNode<K, V...>::Record rec) {
        bool isInserted = this->root->insert(key, rec);
        if (!isInserted) {
            BTreeNode<K, V...> *left, *right;
            K pushedKey = this->root->splitSelf(&left, &right);
            this->root =
                new InterNode<K, V...>(degree, {pushedKey}, {left, right});
        }
    }

    void display() {
        std::cout << "Display BTree:" << std::endl;

        std::queue<std::pair<BTreeNode<K, V...> *, int>> q;
        q.push({this->root, 0});

        while (!q.empty()) {
            auto [node, lev] = q.front();
            q.pop();

            std::cout << "Level: " << lev << ", ";

            std::cout << "Node: ";
            for (auto key : node->keys) {
                std::cout << key << " ";
            }
            std::cout << " | \n";

            for (auto child : node->children) {
                if (child != nullptr) {
                    q.push({child, lev + 1});
                }
            }
        }
    }
};

}  // namespace DB

int main() {
    DB::BTree<int, int> btree(3);
    btree.insert(1, {1, 1});
    btree.display();
    btree.insert(2, {2, 2});
    btree.display();
    btree.insert(3, {3, 3});
    btree.display();
    btree.insert(4, {4, 4});
    btree.display();
    btree.insert(5, {5, 5});
    btree.display();
    btree.insert(6, {6, 6});
    btree.display();
    btree.insert(7, {7, 7});
    btree.display();
    btree.insert(8, {8, 8});
    btree.display();
    btree.insert(9, {9, 9});
    btree.display();
    btree.insert(10, {10, 10});
    btree.display();

    return 0;
}