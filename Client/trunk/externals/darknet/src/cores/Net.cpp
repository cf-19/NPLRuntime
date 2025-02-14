﻿#include "Net.h"
#include "Options.h"
#include "utils/blas.h"
#include "utils/utils.h"

#include "layers/ConnectedLayer.h"
#include "layers/ConvolutionalLayer.h"
#include "layers/DropOutLayer.h"
#include "layers/MaxPoolLayer.h"
#include "layers/RouteLayer.h"
#include "layers/ShortCutLayer.h"
#include "layers/SoftMaxLayer.h"
#include "layers/UpSampleLayer.h"
#include "layers/YoloLayer.h"
namespace darknet
{
    Net::Net()
    {
        m_in_datas  = nullptr;
        m_in_truths = nullptr;
        m_batch     = 1;
    }

    std::shared_ptr<Layer> Net::MakeLayer(Layer *prev_layer, std::shared_ptr<Options> layer_options)
    {
        std::shared_ptr<Layer> layer = nullptr;
        auto layer_type              = Layer::GetLayerTypeByName(layer_options->GetName());
        if (layer_type == Layer::s_convolutional)
        {
            layer = std::make_shared<ConvolutionalLayer>(this, prev_layer, layer_options);
        }
        else if (layer_type == Layer::s_connected)
        {
            layer = std::make_shared<ConnectedLayer>(this, prev_layer, layer_options);
        }
        else if (layer_type == Layer::s_maxpool)
        {
            layer = std::make_shared<MaxPoolLayer>(this, prev_layer, layer_options);
        }
        else if (layer_type == Layer::s_route)
        {
            layer = std::make_shared<RouteLayer>(this, prev_layer, layer_options);
        }
        else if (layer_type == Layer::s_shortcut)
        {
            layer = std::make_shared<ShortCutLayer>(this, prev_layer, layer_options);
        }
        else if (layer_type == Layer::s_upsample)
        {
            layer = std::make_shared<UpSampleLayer>(this, prev_layer, layer_options);
        }
        else if (layer_type == Layer::s_dropout)
        {
            layer = std::make_shared<DropOutLayer>(this, prev_layer, layer_options);
        }
        else if (layer_type == Layer::s_softmax)
        {
            layer = std::make_shared<SoftMaxLayer>(this, prev_layer, layer_options);
        }
        else if (layer_type == Layer::s_yolo)
        {
            layer = std::make_shared<YoloLayer>(this, prev_layer, layer_options);
        }
        else
        {
            std::cerr << "无效层: " << layer_options->GetName() << std::endl;
            return nullptr;
        }

        layer->SetBatchNormalize(m_batch_normalize);
        layer->SetBatch(m_batch);
        layer->Resize();

        auto layer_workspace_size = layer->GetWorkspaceSize();
        if (m_workspace.size() < layer_workspace_size) m_workspace.resize(layer_workspace_size);

        return layer;
    }

    bool Net::Load(std::shared_ptr<std::vector<std::shared_ptr<Options>>> options_list)
    {
        if (options_list == nullptr || options_list->size() == 0) return false;
        m_layers.clear();
        m_options = options_list->front();

        auto net_options  = GetOptions();
        m_width           = net_options->GetOption("width", 96);  // 3 * 32
        m_height          = net_options->GetOption("height", 96); // 3 * 32
        m_channels        = net_options->GetOption("channels", 3);
        m_batch_normalize = net_options->GetOption("batch_normalize", false);

        auto prev_layer = GetFrontLayer();
        int layers_size = options_list->size() - 1;

        for (int i = 1; i <= layers_size; i++)
        {
            auto layer_options = options_list->at(i);
            auto layer         = MakeLayer(prev_layer, layer_options);
            if (layer->GetType() == Layer::s_yolo)
            {
                if (layer->GetInSize() != layer->GetOutSize())
                {
                    auto unit_conv_options = std::make_shared<Options>("convolutional");
                    unit_conv_options->SetOption("size", "1");
                    unit_conv_options->SetOption("stride", "1");
                    unit_conv_options->SetOption("padding", "0");
                    unit_conv_options->SetOption("batch_normalize", "0");
                    unit_conv_options->SetOption("filters", std::to_string(layer->GetOutFilters()));
                    unit_conv_options->SetOption("activation", "linear");
                    m_layers.push_back(MakeLayer(prev_layer, unit_conv_options));
                    prev_layer = m_layers.back().get();
                    layer      = MakeLayer(prev_layer, layer_options);
                }
            }
            m_layers.push_back(layer);
            prev_layer = layer.get();
        }

        if (GetBackLayer()->GetType() == Layer::s_yolo)
        {
            m_truths_size = 5;
            m_max_truths  = 10;
        }
        else
        {
            m_truths_size = GetBackLayer()->GetOutSize();
            m_max_truths  = 1;
        }

        m_truths_size = net_options->GetOption("truths_size", m_truths_size);
        m_max_truths  = net_options->GetOption("max_truths", m_max_truths);

        // PrintNetInfo();

        return true;
    }

    void Net::PrintNetInfo(std::ostream *os)
    {
        if (os == nullptr) os = &std::cout;
        *os << std::setw(4) << std::left << "id";
        *os << std::setw(16) << std::left << "     layer";
        *os << std::setw(10) << std::left << " filters";
        *os << std::setw(16) << std::left << "  size/stride";
        *os << std::setw(16) << std::left << "     input";
        *os << "  ->  ";
        *os << std::setw(16) << "     output" << std::left;
        *os << std::setw(8) << " params ";
        *os << std::endl;
        int layers_size = m_layers.size();
        for (int i = 0; i < layers_size; i++)
        {
            auto layer = m_layers[i].get();

            *os << std::setw(4) << std::left << i;
            *os << std::setw(16) << std::left << layer->GetName();

            std::ostringstream filter_oss;
            filter_oss << "    " << layer->GetOutFilters();
            *os << std::setw(10) << std::left << filter_oss.str();

            if (layer->GetType() == Layer::s_convolutional)
            {
                ConvolutionalLayer *conv = (ConvolutionalLayer *)(layer);
                std::ostringstream size_stride_oss;
                size_stride_oss << std::setw(3) << std::right << conv->GetSize() << " x " << std::setw(3) << std::left << conv->GetSize() << "/ " << std::setw(3) << std::left << conv->GetStride();
                *os << std::setw(16) << size_stride_oss.str();
            }
            else
            {
                *os << std::setw(16) << "";
            }

            // input
            std::ostringstream input_oss;
            input_oss << std::setw(3) << std::right << layer->GetInWidth() << " x " << std::setw(3) << std::left << layer->GetInHeight() << " x " << std::setw(3) << std::left << layer->GetInFilters();
            *os << std::setw(16) << input_oss.str();

            // ->
            *os << "  ->  ";

            // output
            std::ostringstream output_oss;
            output_oss << std::setw(3) << std::right << layer->GetOutWidth() << " x " << std::setw(3) << std::left << layer->GetOutHeight() << " x " << std::setw(3) << std::left << layer->GetOutFilters();
            *os << std::setw(16) << output_oss.str();

            // params
            *os << " " << layer->GetWeightsSize() + layer->GetBiasesSize();
            *os << std::endl;
        }
    }

    void Net::Forward()
    {
        auto layer = GetFrontLayer();
        while (layer != nullptr)
        {
            // 清零输出数据
            fill_cpu(layer->GetBatchOutSize(), 0, layer->GetOutDatas(), 1);
            fill_cpu(layer->GetBatchOutSize(), 0, layer->GetInDeltas(), 1);
            layer->Forward();
            layer = layer->GetNextLayer();
        }
    }

    void Net::Backward()
    {
        auto layer = GetBackLayer();
        while (layer != nullptr)
        {
            layer->Backward();
            layer = layer->GetPrevLayer();
        }
    }

    void Net::Update(float learning_rate, float momentum, float decay, int epoch_batch_size)
    {
        auto layer = GetFrontLayer();
        while (layer != nullptr)
        {
            layer->Update(learning_rate, momentum, decay, epoch_batch_size);
            layer = layer->GetNextLayer();
        }
    }

    float Net::Loss()
    {
        auto layer = GetBackLayer();
        return layer == nullptr ? 0.f : layer->Loss();
    }

    void Net::SetBatch(int batch)
    {
        if (m_batch == batch) return;
        m_batch                 = batch;
        auto max_workspace_size = 0;
        auto batch_normalize    = GetBatchNormalize();
        auto layer              = GetFrontLayer();
        while (layer != nullptr)
        {
            layer->SetBatchNormalize(batch_normalize);
            layer->SetBatch(batch);
            layer->Resize();

            int layer_workspace_size = layer->GetWorkspaceSize();
            max_workspace_size       = max_workspace_size < layer_workspace_size ? layer_workspace_size : max_workspace_size;

            layer = layer->GetNextLayer();
        }

        m_workspace.resize(max_workspace_size);
    }

    bool Net::Save(std::ostream &os)
    {
        auto layers = GetLayers();
        std::vector<std::shared_ptr<Options>> options_list;
        options_list.push_back(GetOptions());

        for (auto it = layers->begin(); it != layers->end(); it++)
        {
            options_list.push_back((*it)->GetOptions());
        }

        int options_size = options_list.size();
        os.write((const char *)(&options_size), sizeof(options_size));
        for (int i = 0; i < options_size; i++)
        {
            auto options             = options_list[i];
            std::string options_name = options->GetName();
            int options_name_size    = options_name.size();
            os.write((const char *)(&options_name_size), sizeof(options_name_size));
            os.write(options_name.c_str(), options_name_size);
            auto options_map     = options->GetOptions();
            int options_map_size = options_map->size();
            os.write((const char *)(&options_map_size), sizeof(options_map_size));
            auto options_map_end_it = options_map->end();
            for (auto it = options_map->begin(); it != options_map_end_it; it++)
            {
                auto key       = it->first;
                auto value     = it->second;
                int key_size   = key.size();
                int value_size = value.size();
                os.write((const char *)(&key_size), sizeof(key_size));
                os.write(key.c_str(), key_size);
                os.write((const char *)(&value_size), sizeof(value_size));
                os.write(value.c_str(), value_size);
            }
        }
        int layers_size = m_layers.size();
        for (int i = 0; i < layers_size; i++)
        {
            auto layer = m_layers[i];
            layer->SaveWeights(os);
        }
        return true;
    }

    bool Net::Load(std::istream &is)
    {
        int options_size = 0;
        if (!is.good()) return false;
        is.read((char *)(&options_size), sizeof(options_size));

        auto options_list = std::make_shared<std::vector<std::shared_ptr<Options>>>();
        for (int i = 0; i < options_size; i++)
        {
            if (!is.good()) return false;

            auto options          = std::make_shared<Options>();
            int options_name_size = 0;
            std::string options_name;
            is.read((char *)(&options_name_size), sizeof(options_name_size));
            options_name.resize(options_name_size);

            is.read((char *)(options_name.c_str()), options_name_size);
            options->SetName(options_name);

            int options_map_size = 0;
            is.read((char *)(&options_map_size), sizeof(options_map_size));
            for (int j = 0; j < options_map_size; j++)
            {
                if (!is.good()) return false;

                std::string key;
                int key_size = 0;
                is.read((char *)(&key_size), sizeof(key_size));
                key.resize(key_size);
                is.read((char *)(key.c_str()), key_size);

                std::string value;
                int value_size = 0;
                is.read((char *)(&value_size), sizeof(value_size));
                value.resize(value_size);
                is.read((char *)(value.c_str()), value_size);

                options->SetOption(key, value);
            }
            options_list->push_back(options);
        }

        if (Load(options_list) && is.good())
        {
            int layers_size = m_layers.size();
            for (int i = 0; i < layers_size; i++)
            {
                if (!is.good()) return false;

                auto layer = m_layers[i];
                if (!layer->LoadWeights(is)) return false;
            }
        }

        return true;
    }

    bool Net::Save(const std::string &model_path)
    {
        std::ofstream ofs(model_path, std::ios::binary | std::ios::out);
        if (ofs) return Save(ofs);
        return false;
    }

    bool Net::Load(const std::string &model_path)
    {
        std::ifstream ifs(model_path, std::ios::binary | std::ios::in);
        if (ifs.is_open())
        {
            return Load(ifs);
        }
        else
        {
            std::cerr << "invalid model file: " << model_path << std::endl;
            return false;
        }
    }
}