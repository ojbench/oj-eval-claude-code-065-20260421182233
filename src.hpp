#ifndef SRC_HPP
#define SRC_HPP

#include <vector>
#include <list>

int* getNewBlock(int n);
void freeBlock(const int* block, int n);

class Allocator {
public:
    struct Segment {
        int* ptr;
        int size; // size in number of ints
    };

    Allocator() {}

    ~Allocator() {
        for (auto& b : all_blocks) {
            freeBlock(b.ptr, b.n_blocks);
        }
    }

    int* allocate(int n) {
        if (n <= 0) return nullptr;

        // 1. Try to find a segment in free_list (First-fit)
        for (auto it = free_list.begin(); it != free_list.end(); ++it) {
            if (it->size >= n) {
                int* res = it->ptr;
                if (it->size == n) {
                    free_list.erase(it);
                } else {
                    it->ptr += n;
                    it->size -= n;
                }
                return res;
            }
        }

        // 2. Not found, get new block
        // 4096 bytes is the block size.
        // We need n * sizeof(int) bytes.
        size_t bytes_needed = (size_t)n * sizeof(int);
        int blocks_needed = (int)((bytes_needed + 4095) / 4096);
        int* new_ptr = getNewBlock(blocks_needed);
        all_blocks.push_back({new_ptr, blocks_needed});

        int total_ints = (int)(((size_t)blocks_needed * 4096) / sizeof(int));

        // Use n ints, put the rest in free_list
        if (total_ints > n) {
            insert_free_segment(new_ptr + n, total_ints - n);
        }

        return new_ptr;
    }

    void deallocate(int* pointer, int n) {
        if (!pointer || n <= 0) return;
        insert_free_segment(pointer, n);
    }

private:
    struct BlockRecord {
        int* ptr;
        int n_blocks;
    };
    std::vector<BlockRecord> all_blocks;
    std::list<Segment> free_list;

    void insert_free_segment(int* ptr, int size) {
        // Keep free_list sorted by ptr and merge adjacent segments
        auto it = free_list.begin();
        while (it != free_list.end() && it->ptr < ptr) {
            ++it;
        }

        // Potential merge with previous segment
        bool merged_with_prev = false;
        if (it != free_list.begin()) {
            auto prev = std::prev(it);
            if (prev->ptr + prev->size == ptr) {
                prev->size += size;
                ptr = prev->ptr;
                size = prev->size;
                merged_with_prev = true;
            }
        }

        // Potential merge with next segment
        if (it != free_list.end() && ptr + size == it->ptr) {
            if (merged_with_prev) {
                // Already updated prev, now merge it with current it
                auto prev = std::prev(it);
                prev->size += it->size;
                free_list.erase(it);
            } else {
                // Just merge with next
                it->ptr = ptr;
                it->size += size;
            }
        } else if (!merged_with_prev) {
            // No merge possible with either side
            free_list.insert(it, {ptr, size});
        }
    }
};

#endif
