#include <assert.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>

class List {
 public:
  explicit List(int capacity) : capacity_(capacity),
                                begin_pointer_(0),
                                end_pointer_(0) {
    items_.resize(capacity);
    links_.resize(capacity);
  }

  //  вставляет элемент в список и возвращает позицию вставки
  int insert(int item) {
    int pointer = end_pointer_;
    items_[end_pointer_++] = item;
    end_pointer_ %= capacity_;
    return pointer;
  }

  //  удаляет элемент из списка и возвращает ссылку на элемент,
  //  на который он ссылался
  int remove() {
    int link = links_[begin_pointer_++];
    begin_pointer_ %= capacity_;
    return link;
  }

  void setLink(int position, int link) {
    links_[position] = link;
  }

  void swapLinks(int position1, int position2) {
    std::swap(links_[position1], links_[position2]);
  }

 private:
  size_t capacity_;
  int begin_pointer_;
  int end_pointer_;
  std::vector<int> items_;
  std::vector<int> links_;
};

class Heap {
 public:
  explicit Heap(bool max) : max_(max),
                            sum_(0) {}

  void insert(int item, int link, List & list, int delta = 0) {
    sum_ += item;
    items_.push_back(item);
    links_.push_back(link);
    list.setLink(link, items_.size() - 1 + delta);                          //  установить ссылку в списке на элемент в куче
    siftUp_(items_.size() - 1, list);
  }

  int pop(int position, List & list) {
    sum_ -= items_[position];
    int link = links_[position];
    swap_nodes(position, items_.size() - 1, list);
    items_.pop_back();
    links_.pop_back();
    if (position != 0 && !correctOrder_((position - 1) / 2, position)) {
      siftUp_(position, list);                                              //  в случае удаления элемента, не являющегося корнем,
                                                                            //  нам может потребоваться поднять вершину, а не опускать
    } else {
      siftDown_(position, list);
    }
    return link;
  }

  size_t getSize() const {
    return items_.size();
  }

  int getMaximum() const {
    return items_.front();
  }

  int getMinimum() const {
    return items_.front();
  }

  int64_t getSum() const {
    return sum_;
  }

 private:
  bool max_;
  int64_t sum_;
  std::vector<int> items_;
  std::vector<int> links_;

  int siftUp_(int position, List & list) {
    while (position != 0 && !correctOrder_((position - 1) / 2, position)) {
      swap_nodes((position - 1) / 2, position, list);
      position = (position - 1) / 2;
    }
    return position;
  }

  void siftDown_(int position, List & list) {
    while (position * 2 + 1 < items_.size()) {
      int child = position * 2 + 1;
      if (position * 2 + 2 < items_.size() &&
        correctOrder_(position * 2 + 2, position * 2 + 1)) {
        child = position * 2 + 2;
      }
      if (correctOrder_(position, child)) {
        break;
      }
      swap_nodes(position, child, list);
      position = child;
    }
  }

  void swap_nodes(int v1, int v2, List & list) {
    list.swapLinks(links_[v1], links_[v2]);                                 //  после обмена элементов ссылки в списке инвалидируются,
                                                                            //  поэтому их нужно поправить
    std::swap(items_[v1], items_[v2]);
    std::swap(links_[v1], links_[v2]);
  }

  bool correctOrder_(int v1, int v2) const {
    if (max_) {
      return items_[v1] >= items_[v2];
    } else {
      return items_[v1] <= items_[v2];
    }
  }
};

class DoubleHeap {
 public:
  explicit DoubleHeap(int capacity) : capacity_max_((capacity + 1) / 2),
                                      list{ capacity },
                                      heap_max_(true),
                                      heap_min_(false) {
  }

  void insert(int item) {
    int linkList = list.insert(item);
    if (heap_max_.getSize() < capacity_max_) {                              //  если ещё можем добавлять в кучу максимумов, добавляем
      heap_max_.insert(item, linkList, list);
    } else if (heap_max_.getMaximum() <= item) {                            //  если добавляемый элемент больше всех в куче максимумов,
      heap_min_.insert(item, linkList, list, capacity_max_);                //  то нужно добавить его в кучу минимумов, учитывая при этом,
                                                                            //  что номер элемента в список нужно будет передать, не забыв про
                                                                            //  элементы в куче максимумов, иначе в списке будут ссылки с
                                                                            //  одинаковыми значениями, ссылающимися на разные кучи
    } else {
      int max = heap_max_.getMaximum();                                     //  иначе нужно перекинуть элемент из кучи максимумов в кучу минимумов
      int link = heap_max_.pop(0, list);                                    //  и тогда добавить в кучу максимумов.
      heap_max_.insert(item, linkList, list);                               //  при этом мы извлекаем максимум из кучи, но не из списка,
      heap_min_.insert(max, link, list, capacity_max_);                     //  поэтому нам нужно сохранить link - ссылку на этот элемент в списке
    }
  }

  void remove() {
    int linkHeap = list.remove();
    if (linkHeap >= capacity_max_) {
      heap_min_.pop(linkHeap - capacity_max_, list);
    } else {
      heap_max_.pop(linkHeap, list);
      if (heap_min_.getSize() != 0) {                                       //  всегда поддерживаем размер кучи максимумов максимально возможным
        int min = heap_min_.getMinimum();
        int link = heap_min_.pop(0, list);
        heap_max_.insert(min, link, list);
      }
    }
  }

  int64_t getPrice() const {
    return static_cast<int64_t>(heap_max_.getMaximum()) * heap_max_.getSize()
         - heap_max_.getSum()
         + heap_min_.getSum()
         - static_cast<int64_t>(heap_max_.getMaximum()) * heap_min_.getSize();
  }

 private:
  size_t capacity_max_;
  Heap heap_max_;
  Heap heap_min_;
  List list;
};

int main() {
  int n;
  int W;
  std::cin >> n >> W;
  DoubleHeap doubleHeap{ W };

  for (int i = 0; i < W; i++) {
    int item;
    std::cin >> item;
    doubleHeap.insert(item);
  }

  int64_t minPrice = doubleHeap.getPrice();

  for (int i = W; i < n; i++) {
    int item;
    std::cin >> item;
    doubleHeap.remove();
    doubleHeap.insert(item);
    minPrice = std::min(minPrice, doubleHeap.getPrice());
  }

  std::cout << minPrice << std::endl;
  return 0;
}
