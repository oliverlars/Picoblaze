
unsigned
hash(String str){
    unsigned long hash = 5381;
    
    int c;
    for(int i = 0; i < str.len; i++){
        c = str.text[i];
        hash = ((hash << 5) + hash) + c;
    }
    return hash % HASH_SIZE;
}

static Hash_Node*
alloc_node(int value){
    
    Hash_Node* node = (Hash_Node*)malloc(sizeof(Hash_Node));
    node->value = value;
    node->next = nullptr;
    return node;
}

static void
map_insert(Map& map, String key, int value){
    
    int index = hash(key);
    
    if(!map.nodes[index]){
        map.nodes[index] = alloc_node(value);
    }else{
        Hash_Node* head = map.nodes[index];
        Hash_Node* node = alloc_node(value);
        node->next = head;
        map.nodes[index] = node;
    }
}

static int
map_get(Map& map, String key){
    
    Hash_Node* node = map.nodes[hash(key)];
    if(!node) return -1;
    return node->value;
}

static void
print_map(Map& map){
    for(int i = 0; i < HASH_SIZE; i++){
        if(map.nodes[i]){
            printf("%d\n", map.nodes[i]->value);
        }else{
            printf("0\n");
        }
    }
}