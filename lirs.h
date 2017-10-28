#define likely(x) __builtin_expect(!!(x), 1) //gcc内置函数, 帮助编译器分支优化
//#define unlikely(x) __builtin_expect(!!(x), 0)

#define DB 0
#define STACK_LIMIT 1

struct lirs_entry
{
    enum LIRS_TYPE type;
    struct list_entry *s; //stack指针
    struct list_entry *q; //q指针
	u_int64_t access_cnt;
};

class __LIRS : public Algorithm
{
private:
    struct list_entry *QHEAD; //队头部不存东西
    struct list_entry *SHEAD; //栈头部不存东西
	unordered_map<string,struct lirs_entry *> lirs_map; //Q list索引
	u_int64_t lir_num_conf;
	u_int64_t hir_num_conf;
	u_int64_t stack_size_conf;
	u_int64_t current_num; //当前缓存空间
	u_int64_t lir_num;	//lir的数量
	u_int64_t hir_num;	//hir resident数量
	u_int64_t stack_size; //stack的大小

public: //缓存基本操作

    //初始化
    __LIRS()
    {
    	QHEAD = (struct list_entry *)malloc(sizeof(struct list_entry));
    	SHEAD = (struct list_entry *)malloc(sizeof(struct list_entry));

    	QHEAD->next = QHEAD->pre = QHEAD;
		SHEAD->next = SHEAD->pre = SHEAD;
		
		stack_size_conf = stack_size = lir_num_conf = hir_num_conf = lir_num = hir_num = current_num = 0;

    }

    ~__LIRS()
    {
        //销毁map
        lirs_map.clear();
    }

    void map_operation(u_int64_t key, cache_c *ctx,char *map_key)
    {
    	//cout << hir_num_conf << " " << ctx->block_num_conf << " " << ctx->hir_num_conf << " " << stack_size_conf << endl;
		if(lir_num_conf == 0)
		{
			//hir_num_conf = ctx->block_num_conf << 7;
			hir_num_conf = ctx->block_num_conf * ctx->hir_num_conf;
			lir_num_conf = ctx->block_num_conf - hir_num_conf;
			stack_size_conf = ctx->block_num_conf * ctx->stack_size_conf;
		}

		stack_size_adjust();

        auto got = lirs_map.find(map_key);
        
        if(got == lirs_map.end())
		{//情况3-(2)
			struct lirs_entry * lie = (struct lirs_entry *)malloc(sizeof(struct lirs_entry));
			lie->s = lie->q = NULL;
			lie->access_cnt = 1;
			lirs_map[map_key] = lie;

			//we load the requested block X into the freed buffer and place it on the top of stack S
			struct list_entry *le_s = (struct list_entry *)malloc(sizeof(struct list_entry));
			strcpy(le_s->map_key,map_key);
			lie->s = le_s;
			stack_size++;
			add_s(le_s);

			if(lir_num < lir_num_conf)
			{//When LIR block set is not full, all the referenced blocks are
			//given an LIR status until its size reaches Llirs.
				current_num++;
				lie->type = LIR;
				lir_num++;
				//cout << current_num << " " << lir_num << " " << hir_num << endl;
				return;
			}
			if(hir_num == hir_num_conf)
			{
				//We remove the HIR resident block at the front of list Q
				//it then becomes a non-resident block
				//replace it out of the cache. Then we load the requested block X into the freed buffer and place it on the top of stack S
				struct list_entry *le_tmp = remove_from_q(QHEAD->pre);
				auto ito = lirs_map.find(le_tmp->map_key);
				free(le_tmp);
				le_tmp = NULL;
				ito->second->q = NULL;
				if(ito->second->s == NULL)
				{
					lirs_map.erase(ito);
				}
				hir_num--;
				current_num--;
			}
			//we leave its status in HIR
			lie->type = HIR;
			hir_num++;
			current_num++;
			//place it in the end of list Q
			struct list_entry *le_q = (struct list_entry *)malloc(sizeof(struct list_entry));
			strcpy(le_q->map_key,map_key);
			lie->q = le_q;
			add_q(le_q);
			//cout << map_key << endl;
			//cout << current_num << " " << lir_num << " " << hir_num << endl;
        }
        else
        {
			struct lirs_entry *lie = got->second;
			lie->access_cnt++;
            if(lie->type == LIR)
			{//情况1 Upon accessing an LIR block X
				//This access is guaranteed to be a hit in the cache
                ctx->stat->hit_num++;
                lie->access_cnt++;

				//We move it to the top of stack S
				move_to_SHEAD(lie->s);
				//If the LIR block is originally located in the bottom of the stack, we conduct a stack pruning
                stack_pruning();
            }
            else
            {
	            if(lie->q != NULL)
				{//情况2 Upon accessing an HIR resident block X
					//This is a hit in the cache
	            	ctx->stat->hit_num++;
	            	lie->access_cnt++;

	                if(lie->s != NULL)
					{//情况2-(1) If X is in the stack S
						//we change its status to LIR
						lie->type = LIR;
						//We move it to the top of stack S
	                	move_to_SHEAD(lie->s);
						struct list_entry *le_tmp;

						//This block is also removed from list Q
						le_tmp = remove_from_q(lie->q);
						free(le_tmp);
						le_tmp = NULL;
						lie->q = NULL;

						//The LIR block in the bottom of S is moved to the end of list Q
						le_tmp = remove_from_s(SHEAD->pre);
						stack_size--;
	                	add_q(le_tmp);
	                	auto got_tmp = lirs_map.find(le_tmp->map_key);
	                	if(likely(got_tmp != lirs_map.end()))
	                	{
							auto lie_tmp = got_tmp->second;
							//with its status changed to HIR
	                		lie_tmp->type = HIR;
	                		lie_tmp->q = lie_tmp->s;
	                		lie_tmp->s = NULL;
	                	}
	                	else
	                	{
	                		printf("fatal error\n");
						}
						//A stack pruning is then conducted
	                	stack_pruning();
	                }
	                else
	                {//情况2-(2) If X is not in stack S
	                	struct list_entry *le_tmp = (struct list_entry *)malloc(sizeof(struct list_entry));
	                	struct list_entry *le_tmp2 = lie->q;
	                	strcpy(le_tmp->map_key,le_tmp2->map_key);

						//we leave its status in HIR and move it to the end of list Q
						move_to_QHEAD(lie->q);
						//We move it to the top of stack S
	                	add_s(le_tmp);
						lie->s = le_tmp;
						stack_size++;
	       	        }

	            }
	            else
				{//情况3 Upon accessing an HIR non-resident block X

					//We remove the HIR resident block at the front of list Q
					//it then becomes a non-resident block
					//replace it out of the cache. Then we load the requested block X into the freed buffer and place it on the top of stack S
					struct list_entry *le_tmp = remove_from_q(QHEAD->pre);
					auto ito = lirs_map.find(le_tmp->map_key);
					free(le_tmp);
					le_tmp = NULL;
					ito->second->q = NULL;
					if(ito->second->s == NULL)
					{
						lirs_map.erase(ito);
						//hir_num--;
					}

	                if(likely(lie->s != NULL))
					{//情况3-(1)If X is in stack S
						//we change its status to LIR
						lie->type = LIR;
						//we load the requested block X into the freed buffer and place it on the top of stack S
						move_to_SHEAD(lie->s);

						// move the LIR block in the bottom of stack S to the end of list Q
						stack_size--;
						le_tmp = remove_from_s(SHEAD->pre);
						auto ito2 = lirs_map.find(le_tmp->map_key);
						add_q(le_tmp);
						ito2->second->q = le_tmp;
						ito2->second->s = NULL;

						//with its status changed to HIR
						ito2->second->type = HIR;

						//A stack pruning is then conducted
						stack_pruning();
	                }
	                else
					{
						printf("fatal error\n");
	                }
	            }
        	}
        }

#if DB
		cout << current_num << " " << lir_num << " " << hir_num << " " << lirs_map.size() << endl;
		list_entry *lte = SHEAD->next;
		while(lte != SHEAD)
		{
			cout << lte->map_key << "|||";
			lte = lte->next;
		}
		cout << endl;
		lte = QHEAD->next;
		while(lte != QHEAD)
		{
			cout << lte->map_key << "|||";
			lte = lte->next;
		}
		cout << endl;
#endif

    }
    
    void stack_pruning()
    {//We define an operation called stack pruning on the LIRS stack S, which removes the HIR blocks in the bottom of
		//the stack until an LIR block sits in the stack bottom.

		//This operation serves for two purposes: 
		//(1) We ensure the block in the bottom of the stack always belongs to the LIR block set.
		//(2) After the LIR block in the bottom is removed, those HIR blocks contiguously located above it will not have chances to
		//change their status from HIR to LIR, because their recencies are larger than the new maximum recency of LIR blocks.
		auto it = lirs_map.find(SHEAD->pre->map_key);
		while(it->second->type != LIR)
		{
			stack_size--;
			remove_from_s(it->second->s);
			free(it->second->s);
			if(it->second->q != NULL)
			{
				hir_num--;
				remove_from_q(it->second->q);
				free(it->second->q);
			}
			lirs_map.erase(it);
			//hir_num--;
			it = lirs_map.find(SHEAD->pre->map_key);
		}
    }

    struct list_entry *remove_from_q(struct list_entry *le)
    {
		le->pre->next = le->next;
		le->next->pre = le->pre;
		le->next = le->pre = NULL;
    	return le;
    }

    void add_q(struct list_entry * le)
    {
		le->next = QHEAD->next;
		QHEAD->next->pre = le;
		QHEAD->next = le;
		le->pre = QHEAD;
    }

    struct list_entry * remove_from_s(struct list_entry *le)
    {
		le->pre->next = le->next;
		le->next->pre = le->pre;
		le->next = le->pre = NULL;
    	return le;
    }

    void add_s(struct list_entry *le)
    {
		le->next = SHEAD->next;
		SHEAD->next->pre = le;
		SHEAD->next = le;
		le->pre = SHEAD;
    }

    void move_to_QHEAD(struct list_entry *le)
    {
    	add_q(remove_from_q(le));
    }

    void move_to_SHEAD(struct list_entry *le)
    {
    	add_s(remove_from_s(le));
	}
	
	void stack_size_adjust()
	{
#if STACK_LIMIT
		list_entry *le = SHEAD->pre;
		while(stack_size > stack_size_conf)
		{
			auto it = lirs_map.find(le->map_key);
			le = le->pre;
			if(it->second->type == LIR)
			{
				continue;
			}

			free(remove_from_s(le->next));
			it->second->s = NULL;
			if(it->second->q != NULL)
			{
				hir_num--;
				free(remove_from_q(it->second->q));
				it->second->q = NULL;
			}
			lirs_map.erase(it);
			stack_size--;
		}
#endif
	}
};