#include "utility.hpp"
#include <functional>
#include <cstddef>
#include "exception.hpp"
#include <map>
namespace sjtu {
	template <class Key, class Value, class Compare = std::less<Key> >
	class BTree {
	private:
		class node {
			friend class BTree;
			Key *key;
			off_t *son,start;
			bool *isleaf;
			public:
				node(const int size,off_t pos):start(pos),key(new Key[size<<1]),son(new off_t[size<<1]),isleaf(new bool[size<<1]) {}
				~node() {delete [] key;delete [] son;delete [] isleaf;}
		};
		class leaf {
			friend class BTree;
			Value value;
			off_t next,prev,start;
			public:
				leaf(off_t pos):start(pos) {}
		};
		const int M;
		mutable FILE *fp;
		mutable int fp_level;
		mutable off_t fp_len;
		char *path;
		void openfile() const {
			if (fp_level++==0) fp=fopen(path,"rb+");
			if (!fp) fp=fopen(path,"w"),fclose(fp),fp=fopen(path,"rb+");
		}
		void closefile() const {if (--fp_level==0) fclose(fp);}
	public:
		typedef pair<const Key, Value> value_type;
		class const_iterator;
		class iterator {
		friend class const_iterator;
		private:
			mutable FILE *fp;
			mutable int fp_level;
			BTree *pos_BTree;
			off_t pos_leaf;
			void openfile() const {
				if (fp_level++==0) fp=fopen(pos_BTree->path,"rb+");
				if (!fp) fp=fopen(path,"w"),fclose(fp),fp=fopen(path,"rb+");
			}
			void closefile() const {if (--fp_level==0) fclose(fp);}
		public:
			bool modify(const Value& value){
				if (pos_leaf<0 || pos_leaf==(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(pos_BTree->M<<1)) throw invalid_iterator();
				Value tem;
				openfile();
				fseek(fp,pos_leaf,0);
				fread(&tem,sizeof(Value),1,fp);
				if (tem==value) {closefile();return false;}
				fseek(fp,-sizeof(Value),1);
				fwrite(&value,sizeof(Value),1,fp);
				closefile();
				return true;
			}
			iterator():fp_level(0),pos_BTree(NULL),pos_leaf(-1) {}
			iterator(const iterator& other):fp_level(other.fp_level),pos_BTree(other.pos_BTree),pos_leaf(other.pos_leaf) {}
			iterator(BTree *p,off_t pos):fp_level(0),pos_BTree(p),pos_leaf(pos) {}
			iterator operator++(int) {
				iterator res=*this;
				++*this;
				return res;
			}
			iterator& operator++() {
				if (pos_leaf<0 || pos_leaf==(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(pos_BTree->M<<1)) throw invalid_iterator();
				openfile();
				fseek(fp,pos_leaf+sizeof(Value),0);
				fread(&pos_leaf,sizeof(off_t),1,fp);
				closefile();
				return *this;
			}
			iterator operator--(int) {
				iterator res=*this;
				--*this;
				return res;
			}
			iterator& operator--() {
				if (pos_leaf<0) throw invalid_iterator();
				off_t tem;
				openfile();
				fseek(fp,pos_leaf+sizeof(Value)+sizeof(off_t),0);
				fread(&tem,sizeof(off_t),1,fp);
				closefile();
				if (tem<0) throw invalid_iterator();
				pos_leaf=tem;
				return *this;
			}
			bool operator==(const iterator& rhs) const {return const_iterator(*this)==const_iterator(rhs);}
			bool operator==(const const_iterator& rhs) const {return const_iterator(*this)==rhs;}
			bool operator!=(const iterator& rhs) const {return !(*this==rhs);}
			bool operator!=(const const_iterator& rhs) const {return !(*this==rhs);}
		};
		class const_iterator {
		friend class iterator;
		private:
			mutable FILE *fp;
			mutable int fp_level;
			const BTree *pos_BTree;
			off_t pos_leaf;
			void openfile() const {
				if (fp_level++==0) fp=fopen(pos_BTree->path,"rb+");
				if (!fp) fp=fopen(path,"w"),fclose(fp),fp=fopen(path,"rb+");
			}
			void closefile() const {if (--fp_level==0) fclose(fp);}
		public:
			const_iterator():fp_level(0),pos_BTree(NULL),pos_leaf(-1) {}
			const_iterator(const const_iterator& other):fp_level(other.fp_level),pos_BTree(other.pos_BTree),pos_leaf(other.pos_leaf) {}
			const_iterator(const iterator& other):fp_level(other.fp_level),pos_BTree(other.pos_BTree),pos_leaf(other.pos_leaf) {}
			const_iterator(const BTree *p,off_t pos):fp_level(0),pos_BTree(p),pos_leaf(pos) {}
			const_iterator operator++(int) {
				const_iterator res=*this;
				++*this;
				return res;
			}
			const_iterator& operator++() {
				if (pos_leaf<0 || pos_leaf==(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(pos_BTree->M<<1)) throw invalid_iterator();
				openfile();
				fseek(fp,pos_leaf+sizeof(Value),0);
				fread(&pos_leaf,sizeof(off_t),1,fp);
				closefile();
				return *this;
			}
			const_iterator operator--(int) {
				const_iterator res=*this;
				--*this;
				return res;
			}
			const_iterator& operator--() {
				if (pos_leaf<0) throw invalid_iterator();
				off_t tem;
				openfile();
				fseek(fp,pos_leaf+sizeof(Value)+sizeof(off_t),0);
				fread(&tem,sizeof(off_t),1,fp);
				closefile();
				if (tem<0) throw invalid_iterator();
				pos_leaf=tem;
				return *this;
			}
			bool operator==(const iterator& rhs) const {return *this==const_iterator(rhs);}
			bool operator==(const const_iterator& rhs) const {
				if (pos_leaf!=rhs.pos_leaf) return false;
				int i;
				for (i=0;pos_BTree->path[i] && rhs.pos_BTree->path[i];++i)
					if (pos_BTree->path[i]!=rhs.pos_BTree->path[i]) return false;
				return pos_BTree->path[i]=='\0' && rhs.pos_BTree->path[i]=='\0';
			}
			bool operator!=(const iterator& rhs) const {return !(*this==rhs);}
			bool operator!=(const const_iterator& rhs) const {return !(*this==rhs);}
		};
		BTree():M(2048/sizeof(Key)),fp_level(0),path(new char[9]) {
			path[0]='t';path[1]='e';path[2]='s';path[3]='t';path[4]='.';path[5]='t';path[6]='x';path[7]='t';path[8]='\0';
			openfile();
			fseek(fp,0,2);
			if (ftell(fp)==0)
			{
				node *root=new node(M,0);
				int i;
				for (i=0;i<(M<<1);++i) root->son[i]=-1;
				fseek(fp,0,0);
				fwrite(root->key,sizeof(Key),M<<1,fp);
				fwrite(root->son,sizeof(off_t),M<<1,fp);
				fwrite(root->isleaf,sizeof(bool),M<<1,fp);
				leaf *tail=new leaf((sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1));
				tail->next=tail->prev=-1;
				fwrite(&tail->value,sizeof(Value),1,fp);
				fwrite(&tail->next,sizeof(off_t),1,fp);
				fwrite(&tail->prev,sizeof(off_t),1,fp);
				size_t num=0;
				fwrite(&num,sizeof(size_t),1,fp);
				delete root;delete tail;
			}
			fp_len=ftell(fp);
			closefile();
		}
		BTree(const BTree& other):M(other.M),fp_level(other.fp_level),fp_len(other.fp_len) {
			int len=0;
			for (;other.path[len];++len);
			for (path=new char[len+1];len>=0;--len) path[len]=other.path[len];
		}
		BTree& operator=(const BTree& other) {
			fp_level=other.fp_level;fp_len=other.fp_len;
			int len=0;
			for (;other.path[len];++len);
			for (delete [] path,path=new char[len+1];len>=0;--len) path[len]=other.path[len];
		}
		~BTree() {for (delete [] path;fp_level;closefile());}
		pair<iterator, OperationResult> insert(const Key& key, const Value& value) {
			int i;
			openfile();
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			size_t num;
			fread(&num,sizeof(size_t),1,fp);
			if (num==0)
			{
				node *root=new node(M,0);
				root->key[0]=key;root->son[0]=fp_len;root->isleaf[0]=true;
				for (i=1;i<(M<<1);++i) root->son[i]=-1;
				fseek(fp,0,0);
				fwrite(root->key,sizeof(Key),M<<1,fp);
				fwrite(root->son,sizeof(off_t),M<<1,fp);
				fwrite(root->isleaf,sizeof(bool),M<<1,fp);
				leaf *tem=new leaf(fp_len);
				tem->value=value;tem->next=(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1);tem->prev=-1;
				fseek(fp,tem->start,0);
				fwrite(&tem->value,sizeof(Value),1,fp);
				fwrite(&tem->next,sizeof(off_t),1,fp);
				fwrite(&tem->prev,sizeof(off_t),1,fp);
				fseek(fp,tem->next+sizeof(Value)+sizeof(off_t),0);
				fwrite(&tem->start,sizeof(off_t),1,fp);
				num=1;
				fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
				fwrite(&num,sizeof(size_t),1,fp);
				closefile();
				fp_len+=sizeof(Value)+(sizeof(off_t)<<1);
				iterator res=iterator(this,tem->start);
				delete root;delete tem;
				return pair<iterator,OperationResult>(res,Success);
			}
			node *p[10];
			int top=0;
			for (p[0]=new node(M,0);;++top,p[top]=new node(M,p[top-1]->son[i]))
			{
				fseek(fp,p[top]->start,0);
				fread(p[top]->key,sizeof(Key),M<<1,fp);
				fread(p[top]->son,sizeof(off_t),M<<1,fp);
				fread(p[top]->isleaf,sizeof(bool),M<<1,fp);
				if (Compare()(key,p[top]->key[0]))
					if (p[top]->isleaf[0]) {i=-1;break;}
					else {p[top]->key[i=0]=key;fseek(fp,p[top]->start,0);fwrite(p[top]->key,sizeof(Key),M<<1,fp);continue;}
				for (i=0;i<(M<<1)-1 && !Compare()(key,p[top]->key[i+1]) && p[top]->son[i+1]>=0;++i);
				if (!Compare()(p[top]->key[i],key))
				{
					for (closefile();top>=0;) delete p[top--];
					return pair<iterator,OperationResult>(end(),Fail);
				}
				if (p[top]->isleaf[i]) break;
			}
			leaf *tem=new leaf(fp_len);
			tem->value=value;
			if (i<(M<<1)-1 && p[top]->son[i+1]>=0) tem->next=p[top]->son[i+1];
			else fseek(fp,p[top]->son[i]+sizeof(Value),0),fread(&tem->next,sizeof(off_t),1,fp);
			if (i>=0) tem->prev=p[top]->son[i];
			else fseek(fp,p[top]->son[0]+sizeof(Value)+sizeof(off_t),0),fread(&tem->prev,sizeof(off_t),1,fp);
			fseek(fp,tem->start,0);
			fwrite(&tem->value,sizeof(Value),1,fp);
			fwrite(&tem->next,sizeof(off_t),1,fp);
			fwrite(&tem->prev,sizeof(off_t),1,fp);
			fseek(fp,tem->next+sizeof(Value)+sizeof(off_t),0);
			fwrite(&tem->start,sizeof(off_t),1,fp);
			if (tem->prev>=0) fseek(fp,tem->prev+sizeof(Value),0),fwrite(&tem->start,sizeof(off_t),1,fp);
			++num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fwrite(&num,sizeof(size_t),1,fp);
			fp_len+=sizeof(Value)+(sizeof(off_t)<<1);
			Key tem_key=key;
			off_t tem_son=tem->start;
			bool tem_isleaf=true;
			iterator res=iterator(this,tem->start);
			for (delete tem;top>=0;--top)
				if (p[top]->son[(M<<1)-1]<0)
				{
					for (i=(M<<1)-2;p[top]->son[i]<0;--i);
					for (;i>=0 && Compare()(tem_key,p[top]->key[i]);--i) p[top]->key[i+1]=p[top]->key[i],p[top]->son[i+1]=p[top]->son[i],p[top]->isleaf[i+1]=p[top]->isleaf[i];
					++i;p[top]->key[i]=tem_key;p[top]->son[i]=tem_son;p[top]->isleaf[i]=tem_isleaf;
					fseek(fp,p[top]->start,0);
					fwrite(p[top]->key,sizeof(Key),M<<1,fp);
					fwrite(p[top]->son,sizeof(off_t),M<<1,fp);
					fwrite(p[top]->isleaf,sizeof(bool),M<<1,fp);
					for (closefile();top>=0;) delete p[top--];
					return pair<iterator,OperationResult>(res,Success);
				}
				else
				{
					if (top==0) p[0]->start=fp_len,fp_len+=(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1);
					node *tem_node=new node(M,fp_len);
					if (Compare()(tem_key,p[top]->key[M-1]))
					{
						for (i=0;i<=M;++i) tem_node->key[i]=p[top]->key[i+M-1],tem_node->son[i]=p[top]->son[i+M-1],tem_node->isleaf[i]=p[top]->isleaf[i+M-1];
						for (i=M-2;i>=0 && Compare()(tem_key,p[top]->key[i]);--i) p[top]->key[i+1]=p[top]->key[i],p[top]->son[i+1]=p[top]->son[i],p[top]->isleaf[i+1]=p[top]->isleaf[i];
						++i;p[top]->key[i]=tem_key;p[top]->son[i]=tem_son;p[top]->isleaf[i]=tem_isleaf;
					}
					else
					{
						for (i=0;i<M && Compare()(p[top]->key[i+M],tem_key);++i) tem_node->key[i]=p[top]->key[i+M],tem_node->son[i]=p[top]->son[i+M],tem_node->isleaf[i]=p[top]->isleaf[i+M];
						tem_node->key[i]=tem_key;tem_node->son[i]=tem_son;tem_node->isleaf[i]=tem_isleaf;
						for (++i;i<=M;++i) tem_node->key[i]=p[top]->key[i+M-1],tem_node->son[i]=p[top]->son[i+M-1],tem_node->isleaf[i]=p[top]->isleaf[i+M-1];
					}
					for (i=M;i<(M<<1);++i) p[top]->son[i]=-1;
					for (i=M+1;i<(M<<1);++i) tem_node->son[i]=-1;
					fseek(fp,p[top]->start,0);
					fwrite(p[top]->key,sizeof(Key),M<<1,fp);
					fwrite(p[top]->son,sizeof(off_t),M<<1,fp);
					fwrite(p[top]->isleaf,sizeof(bool),M<<1,fp);
					fseek(fp,tem_node->start,0);
					fwrite(tem_node->key,sizeof(Key),M<<1,fp);
					fwrite(tem_node->son,sizeof(off_t),M<<1,fp);
					fwrite(tem_node->isleaf,sizeof(bool),M<<1,fp);
					fp_len+=(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1);
					tem_key=tem_node->key[0];tem_son=tem_node->start;tem_isleaf=false;
					delete tem_node;
					if (top) delete p[top];
				}
			p[0]->key[1]=tem_key;
			p[0]->son[0]=tem_son-(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1);p[0]->son[1]=tem_son;
			p[0]->isleaf[0]=p[0]->isleaf[1]=false;
			for (i=2;i<(M<<1);++i) p[0]->son[i]=-1;
			fseek(fp,0,0);
			fwrite(p[0]->key,sizeof(Key),M<<1,fp);
			fwrite(p[0]->son,sizeof(off_t),M<<1,fp);
			fwrite(p[0]->isleaf,sizeof(bool),M<<1,fp);
			closefile();
			delete p[0];
			return pair<iterator,OperationResult>(res,Success);
		}
		OperationResult erase(const Key& key) {
		  openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			if (num==0) {closefile();return Fail;}
			node *p[10];
			int id[10],i,top=0;
			for (p[0]=new node(M,0);;++top,p[top]=new node(M,p[top-1]->son[i]))
			{
				fseek(fp,p[top]->start,0);
				fread(p[top]->key,sizeof(Key),M<<1,fp);
				fread(p[top]->son,sizeof(off_t),M<<1,fp);
				fread(p[top]->isleaf,sizeof(bool),M<<1,fp);
				if (Compare()(key,p[top]->key[0]))
				{
					for (closefile();top>=0;) delete p[top--];
					return Fail;
				}
				for (i=0;i<(M<<1)-1 && !Compare()(key,p[top]->key[i+1]) && p[top]->son[i+1]>=0;++i);
				id[top]=i;
				if (p[top]->isleaf[i]) break;
			}
			if (Compare()(p[top]->key[i],key))
			{
				for (closefile();top>=0;) delete p[top--];
				return Fail;
			}
			leaf *tem=new leaf(p[top]->son[i]);
			if (i<(M<<1)-1 && p[top]->son[i+1]>=0) tem->next=p[top]->son[i+1];
			else fseek(fp,p[top]->son[i]+sizeof(Value),0),fread(&tem->next,sizeof(off_t),1,fp);
			if (i) tem->prev=p[top]->son[i-1];
			else fseek(fp,p[top]->son[i]+sizeof(Value)+sizeof(off_t),0),fread(&tem->prev,sizeof(off_t),1,fp);
			fseek(fp,tem->next+sizeof(Value)+sizeof(off_t),0);
			fwrite(&tem->prev,sizeof(off_t),1,fp);
			if (tem->prev>=0) fseek(fp,tem->prev+sizeof(Value),0),fwrite(&tem->next,sizeof(off_t),1,fp);
			--num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fwrite(&num,sizeof(size_t),1,fp);
			Key tem_key=key;
			for (delete tem;top>=0;)
			{
				for (i=0;Compare()(p[top]->key[i],tem_key);++i);
				for (;i<(M<<1)-1 && p[top]->son[i+1]>=0;++i) p[top]->key[i]=p[top]->key[i+1],p[top]->son[i]=p[top]->son[i+1],p[top]->isleaf[i]=p[top]->isleaf[i+1];
				p[top]->son[i]=-1;
				if (top==1 && p[0]->son[1]<0) break;
				if (p[top]->son[M-1]>=0 || top==0)
				{
					fseek(fp,p[top]->start,0);
					fwrite(p[top]->key,sizeof(Key),M<<1,fp);
					fwrite(p[top]->son,sizeof(off_t),M<<1,fp);
					fwrite(p[top]->isleaf,sizeof(bool),M<<1,fp);
					for (i=top-1;i>=0;--i)
					{
						if (Compare()(p[i]->key[id[i]],tem_key)) break;
						p[i]->key[id[i]]=p[i+1]->key[0];
						fseek(fp,p[i]->start,0);
						fwrite(p[i]->key,sizeof(Key),M<<1,fp);
					}
					for (closefile();top>=0;) delete p[top--];
					return Success;
				}
				else
				{
					node *tem_node;
					bool flag=false;
					if (id[top-1]<(M<<1)-1 && p[top-1]->son[id[top-1]+1]>=0)
					{
						tem_node=new node(M,p[top-1]->son[id[top-1]+1]);
						fseek(fp,tem_node->start,0);
						fread(tem_node->key,sizeof(Key),M<<1,fp);
						fread(tem_node->son,sizeof(off_t),M<<1,fp);
						fread(tem_node->isleaf,sizeof(bool),M<<1,fp);
						if (tem_node->son[M]>=0)
						{
							p[top]->key[M-1]=tem_node->key[0];p[top]->son[M-1]=tem_node->son[0];p[top]->isleaf[M-1]=tem_node->isleaf[0];
							for (i=0;i<(M<<1)-1 && tem_node->son[i+1]>=0;++i) tem_node->key[i]=tem_node->key[i+1],tem_node->son[i]=tem_node->son[i+1],tem_node->isleaf[i]=tem_node->isleaf[i+1];
							tem_node->son[i]=-1;
							p[top-1]->key[id[top-1]+1]=tem_node->key[0];
							flag=true;
						}
						if (!flag) delete tem_node;
					}
					if (id[top-1] && !flag)
					{
						tem_node=new node(M,p[top-1]->son[id[top-1]-1]);
						fseek(fp,tem_node->start,0);
						fread(tem_node->key,sizeof(Key),M<<1,fp);
						fread(tem_node->son,sizeof(off_t),M<<1,fp);
						fread(tem_node->isleaf,sizeof(bool),M<<1,fp);
						if (tem_node->son[M]>=0)
						{
							for (i=M-1;i;--i) p[top]->key[i]=p[top]->key[i-1],p[top]->son[i]=p[top]->son[i-1],p[top]->isleaf[i]=p[top]->isleaf[i-1];
							for (i=0;i<(M<<1)-1 && tem_node->son[i+1]>=0;++i);
							p[top]->key[0]=tem_node->key[i];p[top]->son[0]=tem_node->son[i];p[top]->isleaf[0]=tem_node->isleaf[i];
							tem_node->son[i]=-1;
							p[top-1]->key[id[top-1]]=p[top]->key[0];
							flag=true;
						}
						if (!flag) delete tem_node;
					}
					if (flag)
					{
						fseek(fp,p[top]->start,0);
						fwrite(p[top]->key,sizeof(Key),M<<1,fp);
						fwrite(p[top]->son,sizeof(off_t),M<<1,fp);
						fwrite(p[top]->isleaf,sizeof(bool),M<<1,fp);
						fseek(fp,tem_node->start,0);
						fwrite(tem_node->key,sizeof(Key),M<<1,fp);
						fwrite(tem_node->son,sizeof(off_t),M<<1,fp);
						fwrite(tem_node->isleaf,sizeof(bool),M<<1,fp);
						fseek(fp,p[top-1]->start,0);
						fwrite(p[top-1]->key,sizeof(Key),M<<1,fp);
						for (closefile();top>=0;) delete p[top--];
						delete tem_node;
						return Success;
					}
					if (id[top-1]<(M<<1)-1 && p[top-1]->son[id[top-1]+1]>=0)
					{
						tem_node=new node(M,p[top-1]->son[id[top-1]+1]);
						fseek(fp,tem_node->start,0);
						fread(tem_node->key,sizeof(Key),M<<1,fp);
						fread(tem_node->son,sizeof(off_t),M<<1,fp);
						fread(tem_node->isleaf,sizeof(bool),M<<1,fp);
						for (i=M-1;i<(M<<1)-1;++i) p[top]->key[i]=tem_node->key[i-M+1],p[top]->son[i]=tem_node->son[i-M+1],p[top]->isleaf[i]=tem_node->isleaf[i-M+1];
						fseek(fp,p[top]->start,0);
						fwrite(p[top]->key,sizeof(Key),M<<1,fp);
						fwrite(p[top]->son,sizeof(off_t),M<<1,fp);
						fwrite(p[top]->isleaf,sizeof(bool),M<<1,fp);
						tem_key=tem_node->key[0];
					}
					else 
					{
						tem_node=new node(M,p[top-1]->son[id[top-1]-1]);
						fseek(fp,tem_node->start,0);
						fread(tem_node->key,sizeof(Key),M<<1,fp);
						fread(tem_node->son,sizeof(off_t),M<<1,fp);
						fread(tem_node->isleaf,sizeof(bool),M<<1,fp);
						for (i=M;i<(M<<1)-1;++i) tem_node->key[i]=p[top]->key[i-M],tem_node->son[i]=p[top]->son[i-M],tem_node->isleaf[i]=p[top]->isleaf[i-M];
						fseek(fp,tem_node->start,0);
						fwrite(tem_node->key,sizeof(Key),M<<1,fp);
						fwrite(tem_node->son,sizeof(off_t),M<<1,fp);
						fwrite(tem_node->isleaf,sizeof(bool),M<<1,fp);
						tem_key=p[top]->key[0];
					}
					delete p[top--];delete tem_node;
				}
			}
			fseek(fp,0,0);
			fwrite(p[1]->key,sizeof(Key),M<<1,fp);
			fwrite(p[1]->son,sizeof(off_t),M<<1,fp);
			fwrite(p[1]->isleaf,sizeof(bool),M<<1,fp);
			for (closefile();top>=0;) delete p[top--];
			return Success;
		}
		iterator begin() {
			openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			if (num==0) {closefile();return end();}
			node *p=new node(M,0);
			for (;;p->start=p->son[0])
			{
				fseek(fp,p->start,0);
				fread(p->key,sizeof(Key),M<<1,fp);
				fread(p->son,sizeof(off_t),M<<1,fp);
				fread(p->isleaf,sizeof(bool),M<<1,fp);
				if (p->isleaf[0]) break;
			}
			closefile();
			iterator res(this,p->son[0]);
			delete p;
			return res;
		}
		const_iterator cbegin() const {
			openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			if (num==0) {closefile();return cend();}
			node *p=new node(M,0);
			for (;;p->start=p->son[0])
			{
				fseek(fp,p->start,0);
				fread(p->key,sizeof(Key),M<<1,fp);
				fread(p->son,sizeof(off_t),M<<1,fp);
				fread(p->isleaf,sizeof(bool),M<<1,fp);
				if (p->isleaf[0]) break;
			}
			closefile();
			const_iterator res(this,p->son[0]);
			delete p;
			return res;
		}
		iterator end() {return iterator(this,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1));}
		const_iterator cend() const {return const_iterator(this,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1));}
		bool empty() const {
			openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			closefile();
			return num==0;
		}
		size_t size() const {
			openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			closefile();
			return num;
		}
		void clear() {
			openfile();
			node *root=new node(M,0);
			int i;
			for (i=0;i<(M<<1);++i) root->son[i]=-1;
			fwrite(root->key,sizeof(Key),M<<1,fp);
			fwrite(root->son,sizeof(off_t),M<<1,fp);
			fwrite(root->isleaf,sizeof(bool),M<<1,fp);
			leaf *tail=new leaf((sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1));
			tail->next=tail->prev=-1;
			fwrite(&tail->value,sizeof(Value),1,fp);
			fwrite(&tail->next,sizeof(off_t),1,fp);
			fwrite(&tail->prev,sizeof(off_t),1,fp);
			size_t num=0;
			fwrite(&num,sizeof(size_t),1,fp);
			fp_len=ftell(fp);
			closefile();
			delete root;delete tail;
		}
		Value at(const Key& key){
			openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			if (num==0) closefile(),throw container_is_empty();
			node *p=new node(M,0);
			int i;
			for (;;p->start=p->son[i])
			{
				fseek(fp,p->start,0);
				fread(p->key,sizeof(Key),M<<1,fp);
				fread(p->son,sizeof(off_t),M<<1,fp);
				fread(p->isleaf,sizeof(bool),M<<1,fp);
				if (Compare()(key,p->key[0])) closefile(),delete p,throw runtime_error();
				for (i=0;i<(M<<1)-1 && !Compare()(key,p->key[i+1]) && p->son[i+1]>=0;++i);
				if (p->isleaf[i]) break;
			}
			if (Compare()(p->key[i],key)) closefile(),delete p,throw runtime_error();
			Value res;
			fseek(fp,p->son[i],0);
			fread(&res,sizeof(Value),1,fp);
			closefile();
			delete p;
			return res;
		}
		size_t count(const Key& key) const {
			openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			if (num==0) {closefile();return 0;}
			node *p=new node(M,0);
			int i;
			for (;;p->start=p->son[i])
			{
				fseek(fp,p->start,0);
				fread(p->key,sizeof(Key),M<<1,fp);
				fread(p->son,sizeof(off_t),M<<1,fp);
				fread(p->isleaf,sizeof(bool),M<<1,fp);
				if (Compare()(key,p->key[0])) {closefile();delete p;return 0;}
				for (i=0;i<(M<<1)-1 && !Compare()(key,p->key[i+1]) && p->son[i+1]>=0;++i);
				if (!Compare()(p->key[i],key)) {closefile();delete p;return 1;}
				if (p->isleaf[i]) {closefile();delete p;return 0;}
			}
		}
		iterator find(const Key& key) {
			openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			if (num==0) {closefile();return end();}
			node *p=new node(M,0);
			int i;
			for (openfile();;p->start=p->son[i])
			{
				fseek(fp,p->start,0);
				fread(p->key,sizeof(Key),M<<1,fp);
				fread(p->son,sizeof(off_t),M<<1,fp);
				fread(p->isleaf,sizeof(bool),M<<1,fp);
				if (Compare()(key,p->key[0])) {closefile();delete p;return end();}
				for (i=0;i<(M<<1)-1 && !Compare()(key,p->key[i+1]) && p->son[i+1]>=0;++i);
				if (p->isleaf[i]) break;
			}
			closefile();
			if (Compare()(p->key[i],key)) {delete p;return end();}
			iterator res(this,p->son[i]);
			delete p;
			return res;
		}
		const_iterator find(const Key& key) const {
			openfile();
			size_t num;
			fseek(fp,(sizeof(Key)+sizeof(off_t)+sizeof(bool))*(M<<1)+sizeof(Value)+(sizeof(off_t)<<1),0);
			fread(&num,sizeof(size_t),1,fp);
			if (num==0) {closefile();return cend();}
			node *p=new node(M,0);
			int i;
			for (openfile();;p->start=p->son[i])
			{
				fseek(fp,p->start,0);
				fread(p->key,sizeof(Key),M<<1,fp);
				fread(p->son,sizeof(off_t),M<<1,fp);
				fread(p->isleaf,sizeof(bool),M<<1,fp);
				if (Compare()(key,p->key[0])) {closefile();delete p;return cend();}
				for (i=0;i<(M<<1)-1 && !Compare()(key,p->key[i+1]) && p->son[i+1]>=0;++i);
				if (p->isleaf[i]) break;
			}
			closefile();
			if (Compare()(p->key[i],key)) {delete p;return cend();}
			const_iterator res(this,p->son[i]);
			delete p;
			return res;
		}
	};
}
