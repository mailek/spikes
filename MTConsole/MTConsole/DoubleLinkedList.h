#pragma once

#define MAX_ITEMS   100

template <typename T>
class CDoubleLinkedList
{
    typedef struct DoubleLinkedListItem
    {
        T*                      item;
        DoubleLinkedListItem*   next;
        DoubleLinkedListItem*   prev;
        bool                    in_use;
    };

public:
    CDoubleLinkedList(void) : first(NULL), last(NULL) {memset(item_array, 0, sizeof(item_array));}
    ~CDoubleLinkedList(void){};

    DoubleLinkedListItem    item_array[MAX_ITEMS];
    DoubleLinkedListItem*   first;
    DoubleLinkedListItem*   last;

public:
    int GetNumberItemsInList()
    {
        int cnt = 0;

        /* count number of items in array that are in use */
        for(int i = 0; i < MAX_ITEMS; i++)
        {
            if(item_array[i].in_use == true)
            {
                cnt++;
            }
        }

        return cnt;
    }

    /* add a new item to the end of the linked list */
    void AddItemToEnd(T* new_item)
    {
        int i;

        /* make sure the item isn't already in use */
        for(i = 0; i < MAX_ITEMS; i++)
        {
            if(item_array[i].item == new_item)
            {
                return;
            }
        }

        /* find a free item record */
        DoubleLinkedListItem *item_record = NULL;

        for(i = 0; i < MAX_ITEMS; i++)
        {
            if(item_array[i].in_use == FALSE)
            {
                item_record = &item_array[i];
                break;
            }
        }

        /* check if out of space in list */
        assert(item_record != NULL);

        /* set up the item record */
        item_record->in_use = true;
        item_record->item = new_item;
        item_record->prev = NULL;
        item_record->next = NULL;

        /* insert record into the list */
        DoubleLinkedListItem* old_last = last;
        last = item_record;
        if(first == NULL)
        {
            first = item_record;
        }

        if(old_last != NULL)
        {
            old_last->next = item_record;
            item_record->prev = old_last;
        }

    }

    /* remove an item from the list, if doesn't exist then return false, otherwise true */
    bool RemoveItem(T* remove_item)
    {
        int i;
        bool return_val = false;

        DoubleLinkedListItem *remove_record = NULL;

        /* look for the item record in the used array */
        for(i = 0; i < MAX_ITEMS; i++)
        {
            if(item_array[i].item == remove_item)
            {
                remove_record = &item_array[i];
                break;
            }
        }

        /* if the record was found, remove from linked list and item array */
        if(remove_record != NULL)
        {
            /* remove from list */
            if(first == remove_record)
            {
                first = remove_record->next;
            }

            if(last == remove_record)
            {
                last = remove_record->prev;
            }

            for(DoubleLinkedListItem* traverse = first; traverse != NULL; traverse = traverse->next)
            {
                if(traverse == remove_record)
                {
                    if(traverse->prev != NULL)
                    {
                        traverse->prev->next = traverse->next;
                    }

                    if(traverse->next != NULL)
                    {
                        traverse->next->prev = traverse->prev;
                    }

                }
            }

            /* remove from array */
            remove_record->in_use = false;
            remove_record->prev = NULL;
            remove_record->next = NULL;

            return_val = true;
        }

        return return_val;
    }

};
