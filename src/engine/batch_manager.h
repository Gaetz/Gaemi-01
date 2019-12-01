#include <vector>
#include <string>
#include <memory>
#include "batch_sprite_vertex.h"
#include "batch_config.h"

using std::vector;
using std::string;

class Batch;

class BatchManager
{

public:
    BatchManager(unsigned uNumBatches, unsigned numVerticesPerBatch);
    virtual ~BatchManager();

    static BatchManager *const get();

    void render(const vector<SpriteVertex>& vVertices, const BatchConfig &config, const std::string &strId);
    void emptyAll();

private:
    std::vector<std::shared_ptr<Batch>> m_vBatches;

    unsigned m_uNumBatches;
    unsigned m_maxNumVerticesPerBatch;

    BatchManager(const BatchManager &c);            // Not Implemented
    BatchManager &operator=(const BatchManager &c); // Not Implemented

    void emptyBatch(bool emptyAll, Batch *pBatchToEmpty);
    //void  renderBatch( const std::vector<GuiVertex>& vVertices, const BatchConfig& config );
};