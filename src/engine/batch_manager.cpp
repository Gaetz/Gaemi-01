#include "batch_manager.h"
#include "batch.h"
#include "log.h"
#include <queue>
static BatchManager *s_pBatchManager = nullptr;

// ----------------------------------------------------------------------------
// BatchManager()
BatchManager::BatchManager(unsigned uNumBatches, unsigned numVerticesPerBatch)
    : m_uNumBatches(uNumBatches),
      m_maxNumVerticesPerBatch(numVerticesPerBatch)
{

    // Test Input Parameters
    if (uNumBatches < 10)
    {
        LOG(Error) << __FUNCTION__ << " uNumBatches{" << uNumBatches << "} is too small.  Choose a number >= 10 ";
        throw;
    }

    // A Good Size For Each Batch Is Between 1-4MB In Size. Number Of Elements That Can Be Stored In A
    // Batch Is Determined By Calculating #Bytes Used By Each Vertex
    if (numVerticesPerBatch < 1000)
    {
        LOG(Error) << __FUNCTION__ << " numVerticesPerBatch{" << numVerticesPerBatch << "} is too small. Choose A Number >= 1000 ";
        throw;
    }

    // Create Desired Number Of Batches
    m_vBatches.reserve(uNumBatches);
    for (unsigned u = 0; u < uNumBatches; ++u)
    {
        m_vBatches.push_back(std::shared_ptr<Batch>(new Batch(numVerticesPerBatch)));
    }

    s_pBatchManager = this;
} // BatchManager

// ----------------------------------------------------------------------------
// ~BatchManager()
BatchManager::~BatchManager()
{
    s_pBatchManager = nullptr;

    m_vBatches.clear();
} // ~BatchManager

// ----------------------------------------------------------------------------
// get()
BatchManager *const BatchManager::get()
{
    if (nullptr == s_pBatchManager)
    {
        LOG(Error) << __FUNCTION__ + std::string(" failed, BatchManager has not been constructed yet");
        throw;
    }
    return s_pBatchManager;
} // get

// ----------------------------------------------------------------------------
// render()
void BatchManager::render(const std::vector<SpriteVertex> &vVertices, const BatchConfig &config, const std::string &strId)
{

    Batch *pEmptyBatch = nullptr;
    Batch *pFullestBatch = m_vBatches[0].get();

    // Determine Which Batch To Put The Vertices Into
    for (unsigned u = 0; u < m_uNumBatches; ++u)
    {
        Batch *pBatch = m_vBatches[u].get();

        if (pBatch->isBatchConfig(config))
        {
            if (!pBatch->isEnoughRoom(vVertices.size()))
            {
                // First Need To Empty This Batch Before Adding Anything To It
                emptyBatch(false, pBatch);

                LOG(Info) << "Forced batch to empty to make room for vertices";
            }
            /*if (s_pSettings->isDebugLoggingEnabled(Settings::DEBUG_RENDER))
            {
                pBatch->addId(strId);
            }*/
            pBatch->add(vVertices);

            return;
        }

        // Store Pointer To First Empty Batch
        if (nullptr == pEmptyBatch && pBatch->isEmpty())
        {
            pEmptyBatch = pBatch;
        }

        // Store Pointer To Fullest Batch
        pFullestBatch = pBatch->getFullest(pFullestBatch);
    }

    // If We Get Here Then We Didn't Find An Appropriate Batch To Put The Vertices Into
    // If We Have An Empty Batch, Put Vertices There
    if (nullptr != pEmptyBatch)
    {
        /*if (s_pSettings->isDebugLoggingEnabled(Settings::DEBUG_RENDER))
        {
            pEmptyBatch->addId(strId);
        }*/
        pEmptyBatch->add(vVertices, config);
        return;
    }

    // No Empty Batches Were Found Therefore We Must Empty One First And Then We Can Use It
    emptyBatch(false, pFullestBatch);
    LOG(Info) << "Forced fullest batch to empty to make room for vertices";

    pFullestBatch->add(vVertices, config);
} // render

// ----------------------------------------------------------------------------
// emptyAll()
void BatchManager::emptyAll()
{
    emptyBatch(true, m_vBatches[0].get());

    LOG(Info) << "Forced all batches to empty";
} // emptyAll

// ----------------------------------------------------------------------------
// CompareBatch
struct CompareBatch : public std::binary_function<Batch *, Batch *, bool>
{
    bool operator()(const Batch *pBatchA, const Batch *pBatchB) const
    {
        return (pBatchA->getPriority() > pBatchB->getPriority());
    } // operator()
};    // CompareFunctor

// ----------------------------------------------------------------------------
// emptyBatch()
// Empties The Batches According To Priority. If emptyAll() Is False Then
// Only Empty The Batches That Are Lower Priority Than The One Specified
// AND Also Empty The One That Is Passed In
void BatchManager::emptyBatch(bool emptyAll, Batch *pBatchToEmpty)
{
    // Sort Bathes By Priority
    std::priority_queue<Batch *, std::vector<Batch *>, CompareBatch> queue;

    for (unsigned u = 0; u < m_uNumBatches; ++u)
    {
        // Add All Non-Empty Batches To Queue Which Will Be Sorted By Order
        // From Lowest To Highest Priority
        if (!m_vBatches[u]->isEmpty())
        {
            if (emptyAll)
            {
                queue.push(m_vBatches[u].get());
            }
            else if (m_vBatches[u]->getPriority() < pBatchToEmpty->getPriority())
            {
                // Only Add Batches That Are Lower In Priority
                queue.push(m_vBatches[u].get());
            }
        }
    }

    // Render All Desired Batches
    while (!queue.empty())
    {
        Batch *pBatch = queue.top();
        pBatch->render();
        queue.pop();
    }

    if (!emptyAll)
    {
        // When Not Emptying All The Batches, We Still Want To Empty
        // The Batch That Is Passed In, In Addition To All Batches
        // That Have Lower Priority Than It
        pBatchToEmpty->render();
    }
}