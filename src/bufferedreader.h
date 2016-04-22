#ifndef buffered_reader__
#define buffered_reader__

#include "readerworker.h"
#include "bufferpool.h"
#include "buffer.h"

#include <fstream>
#include <thread>
#include <mutex>
#include <future>

namespace preproc
{


template<typename Ty>
class BufferedReader 
{

public:
  BufferedReader(size_t bufSize);
  ~BufferedReader();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Open the raw file at path.
  /// \return True if opened, false otherwise.
  ///////////////////////////////////////////////////////////////////////////////
  bool open(const std::string &path);


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Start readering the file.
  ///////////////////////////////////////////////////////////////////////////////
  bool start();


  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Stop reading as soon as possible.
  ///////////////////////////////////////////////////////////////////////////////
  void stop();

  
  ///////////////////////////////////////////////////////////////////////////////
  /// \brief Reset file pointer to begining of file.
  ///////////////////////////////////////////////////////////////////////////////
  void reset();

  bool eof() const {
    return m_future.wait_for(std::chrono::seconds(0)) ==
        std::future_status::ready;
  }

  bool hasNext() const { 
      // if no buffers in pool, check if the reader thread is done.
      // If it hasn't, then we should return true.
      if (!m_pool->hasNext()) {
          // if no buffers in pool, but reader is still reading, then
          // we will have a next buffer soon.
          if (!eof()){ 
              return true;
          } else {
              // if there are no buffers and the reader is done, we are
              // done reading.
              return false;
          }
      } else {
          // if there are buffers in the pool, then return true.
          return true;
      }
  }


  Buffer<Ty>* waitNext() 
  { 
    return m_pool->nextFull(); 
  }   

  void waitReturn(Buffer<Ty>* buf) 
  { 
    return m_pool->returnEmpty(buf); 
  }   
  
private:
  std::string m_path;

  size_t m_bufSizeBytes;

  BufferPool<Ty> *m_pool;  
  std::future<size_t> m_future;
};

template<typename Ty>
BufferedReader<Ty>::BufferedReader(size_t bufSize)
  : m_path{ }
  , m_bufSizeBytes{ bufSize }
  , m_pool{ nullptr }
  , m_future{ }
{ }

template<typename Ty>
BufferedReader<Ty>::~BufferedReader()
{
  if (m_pool) delete m_pool;
} 

template<typename Ty>
bool
BufferedReader<Ty>::open(const std::string &path)
{
  m_path = path;
  m_pool = new BufferPool<Ty>(m_bufSizeBytes, 4);  
  m_pool->allocate();
  return true;

}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
bool
BufferedReader<Ty>::start()
{

  m_future = std::async(std::launch::async, [&]() { 
          ReaderWorker<BufferedReader<Ty>, BufferPool<Ty>, Ty> worker(this, m_pool);
          worker.setPath(m_path);
          return worker(); 
      });
  
  return true;
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::stop()
{
//  m_worker->requestStop();
//  Info() << "Waiting for reader thread to stop.";
//  m_readThread->join();
}


///////////////////////////////////////////////////////////////////////////////
template<typename Ty>
void
BufferedReader<Ty>::reset()
{
//  m_is->clear();
//  m_is->seekg(0, std::ios::beg);
    
}


} // namespace preproc
#endif // ! buffered_reader__
