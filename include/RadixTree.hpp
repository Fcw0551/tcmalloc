#include "./MemoryPool.hpp"
#include <cstring>
//基数树进行优化页号《--》span的映射关系
template <int BITS>//需要多少个bit位进行存储页号，比如2^64次方如果按照4kb一页  那么需要2^20存储，即需要20个bit位进行存储
class RadixTree
{
    private:
    // 新增中间层统一常量
    static const int INTERIOR_BITS = (BITS + 2) / 3;       // 第一、二层共用比特数
    //static const size_t INTERIOR_BITS = 10;              // 第一、二层共用比特数
    
    static const int INTERIOR_LENGTH = 1 << INTERIOR_BITS; // 第一、二层共用数组大小
    static const int LEAF_BITS = BITS - 2 * INTERIOR_BITS; // 第三层比特数
    static const int LEAF_LENGTH = 1 << LEAF_BITS;         // 第三层数组大小
    // 非叶子结点，即一二层
    struct Node{
        Node *ptrs[INTERIOR_LENGTH];
    };
    //叶子结点，即第三层
	struct Leaf{
		void* values[LEAF_LENGTH];
	};
    Node* newNode(){
		static MemoryPool<Node> nodePool;
		Node* result = nodePool.New();
		if (result != NULL)
		{   
            //清空防止数据干扰
			memset(result, 0, sizeof(*result));
		}
		return result;
	}

	Node* root_;//根节点
    
    public:
    typedef uintptr_t Number;//基数树的键值类型，页号

    //构造函数防止隐式类型转换
    explicit RadixTree(){
		root_ = newNode();
	}
    void* get(Number k) const
	{
		const Number k1 = k >> (LEAF_BITS + INTERIOR_BITS);         //第一层对应的下标
		const Number k2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1); //第二层对应的下标
		const Number k3 = k & (LEAF_LENGTH - 1);                    //第三层对应的下标
		//页号超出范围，或映射该页号的空间未开辟
		if ((k >> BITS) > 0 || root_->ptrs[k1] == NULL || root_->ptrs[k1]->ptrs[k2] == NULL)
		{
			return NULL;
		}
		return reinterpret_cast<Leaf*>(root_->ptrs[k1]->ptrs[k2])->values[k3]; //返回该页号对应span的指针
    }
    void set(Number k,void* span){
        assert(k>>BITS==0);//页号必须是一个合法范围
     	const Number k1 = k >> (LEAF_BITS + INTERIOR_BITS);         //第一层对应的下标
		const Number k2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1); //第二层对应的下标
		const Number k3 = k & (LEAF_LENGTH - 1);                    //第三层对应的下标
        ensure(k,1);//确保该页号对应的span的空间是存在的
        reinterpret_cast<Leaf*>(root_->ptrs[k1]->ptrs[k2])->values[k3] = span; //建立该页号与对应span的映射
    }
    //检测[k,k+n-1]的空间是否已经申请
    bool ensure(Number k,size_t n){
        for (Number key = k; key <= k + n - 1;)
		{
			const Number k1 = key >> (LEAF_BITS + INTERIOR_BITS);         //第一层对应的下标
		    const Number k2 = (key >> LEAF_BITS) & (INTERIOR_LENGTH - 1); //第二层对应的下标
		
			if (k1 >= INTERIOR_LENGTH || k2 >= INTERIOR_LENGTH) //下标值超出范围
				return false;
			if (root_->ptrs[k1] == NULL) //第一层k1下标指向的空间未开辟
			{
				//开辟对应空间
				Node* node = newNode();
				if (node == NULL) return false;
				root_->ptrs[k1] = node;
			}
			if (root_->ptrs[k1]->ptrs[k2] == NULL) //第二层i2下标指向的空间未开辟
			{
				//开辟对应空间
				static MemoryPool<Leaf> leafPool;
				Leaf* leaf = leafPool.New();
				if (leaf == NULL) return false;
				memset(leaf, 0, sizeof(*leaf));
				root_->ptrs[k1]->ptrs[k2] = reinterpret_cast<Node*>(leaf);
			}
			key = ((key >> LEAF_BITS) + 1) << LEAF_BITS; //关键优化，直接跳到下一个
		}
		return true;
    }
    void erase(Number k){
        const Number k1 = k >> (LEAF_BITS + INTERIOR_BITS);
        const Number k2 = (k >> LEAF_BITS) & (INTERIOR_LENGTH - 1);
        const Number k3 = k & (LEAF_LENGTH - 1);

        // 检查路径是否存在
        if ((k >> BITS) > 0 || root_->ptrs[k1] == NULL || root_->ptrs[k1]->ptrs[k2] == NULL)
        {
            return ;
        }

        // 只清空映射，不回收节点
        Leaf *leaf = reinterpret_cast<Leaf *>(root_->ptrs[k1]->ptrs[k2]);
        leaf->values[k3] = NULL;

        return ;
    }
};