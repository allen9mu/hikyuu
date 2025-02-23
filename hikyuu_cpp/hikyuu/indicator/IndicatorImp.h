/*
 * ';.kmuhbvcdxIndicatorImp.h
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */

#ifndef INDICATORIMP_H_
#define INDICATORIMP_H_

#include "../config.h"
#include "../KData.h"
#include "../utilities/Parameter.h"
#include "../utilities/util.h"

#if HKU_SUPPORT_SERIALIZATION
    #if HKU_SUPPORT_XML_ARCHIVE
    #include <boost/archive/xml_oarchive.hpp>
    #include <boost/archive/xml_iarchive.hpp>
    #endif /* HKU_SUPPORT_XML_ARCHIVE */

    #if HKU_SUPPORT_TEXT_ARCHIVE
    #include <boost/archive/text_oarchive.hpp>
    #include <boost/archive/text_iarchive.hpp>
    #endif /* HKU_SUPPORT_TEXT_ARCHIVE */

    #if HKU_SUPPORT_BINARY_ARCHIVE
    #include <boost/archive/binary_oarchive.hpp>
    #include <boost/archive/binary_iarchive.hpp>
    #endif /* HKU_SUPPORT_BINARY_ARCHIVE */

#include <boost/serialization/export.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/base_object.hpp>
#endif /* HKU_SUPPORT_SERIALIZATION */


namespace hku {

#define MAX_RESULT_NUM 6

class HKU_API Indicator;

/**
 * 指标实现类，定义新指标时，应从此类继承
 * @ingroup Indicator
 */
class HKU_API IndicatorImp: public enable_shared_from_this<IndicatorImp> {
    PARAMETER_SUPPORT

public:
    enum OPType {
        LEAF, ///<叶子节点
        OP,  /// OP(OP1,OP2) OP1->calcalue(OP2->calculate(ind))
        ADD, ///<加
        SUB, ///<减
        MUL, ///<乘
        DIV, ///<除
        MOD, ///<取模
        EQ,  ///<等于
        GT,  ///<大于
        LT,  ///<小于
        NE,  ///<不等于
        GE,  ///<大于等于
        LE,  ///<小于等于
        AND, ///<与
        OR,  ///<或
        WEAVE, ///<特殊的，需要两个指标作为参数的指标
        IF,  ///if操作
        INVALID
    };

public:
    /** 默认构造函数   */
    IndicatorImp();
    IndicatorImp(const string& name);
    IndicatorImp(const string& name, size_t result_num);

    virtual ~IndicatorImp();

    typedef shared_ptr<IndicatorImp> IndicatorImpPtr;
    virtual IndicatorImpPtr operator()(const Indicator& ind);

    size_t getResultNumber() const;

    size_t discard() const;

    void setDiscard(size_t discard);

    size_t size() const;

    price_t get(size_t pos, size_t num = 0);

    /** 以PriceList方式获取指定的输出集 */
    PriceList getResultAsPriceList(size_t result_num);

    /** 以Indicator的方式获取指定的输出集，该方式包含了discard的信息 */
    IndicatorImpPtr getResult(size_t result_num);

    /**
     * 使用IndicatorImp(const Indicator&...)构造函数后，计算结果使用该函数,
     * 未做越界保护
     */
    void _set(price_t val, size_t pos, size_t num = 0);

    /**
     * 准备内存
     * @param len 长度，如果长度大于MAX_RESULT_NUM将抛出异常std::invalid_argument
     * @param result_num 结果集数量
     * @return true 成功 | false 失败
     */
    void _readyBuffer(size_t len, size_t result_num);

    string name() const;
    void name(const string& name);

    /** 返回形如：Name(param1=val,param2=val,...) */
    string long_name() const;

    string formula() const;

    bool isLeaf() const;

    Indicator calculate();

    void setContext(const Stock&, const KQuery&);

    void setContext(const KData&);

    KData getContext() const;

    void add(OPType, IndicatorImpPtr left, IndicatorImpPtr right);

    void add_if(IndicatorImpPtr cond, IndicatorImpPtr left, IndicatorImpPtr right);

    IndicatorImpPtr clone();

    // ===================
    //  子类接口
    // ===================
    virtual bool check() { return true; }

    virtual void _calculate(const Indicator&) {}

    virtual IndicatorImpPtr _clone() { return make_shared<IndicatorImp>(); }

    virtual bool isNeedContext() const { return false; }

private:
    void initContext();
    bool needCalculate();
    void execute_add();
    void execute_sub();
    void execute_mul();
    void execute_div();
    void execute_mod();
    void execute_eq();
    void execute_ne();
    void execute_gt();
    void execute_lt();
    void execute_ge();
    void execute_le();
    void execute_and();
    void execute_or();
    void execute_weave();
    void execute_if();

protected:
    string m_name;
    size_t m_discard;
    size_t m_result_num;
    PriceList *m_pBuffer[MAX_RESULT_NUM];

    bool m_need_calculate;
    OPType m_optype;
    IndicatorImpPtr m_left;
    IndicatorImpPtr m_right;
    IndicatorImpPtr m_three;

#if HKU_SUPPORT_SERIALIZATION
private:
    friend class boost::serialization::access;
    template<class Archive>
    void save(Archive & ar, const unsigned int version) const {
        namespace bs = boost::serialization;
        string name_str(GBToUTF8(m_name));
        ar & bs::make_nvp<string>("m_name", name_str);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_discard);
        ar & BOOST_SERIALIZATION_NVP(m_result_num);
        ar & BOOST_SERIALIZATION_NVP(m_need_calculate);
        ar & BOOST_SERIALIZATION_NVP(m_optype);
        ar & BOOST_SERIALIZATION_NVP(m_left);
        ar & BOOST_SERIALIZATION_NVP(m_right);
        ar & BOOST_SERIALIZATION_NVP(m_three);
        size_t act_result_num = 0;
        size_t i = 0;
        while (i < m_result_num) {
            if (m_pBuffer[i++])
                act_result_num++;
        }
        ar & BOOST_SERIALIZATION_NVP(act_result_num);

        for (size_t i = 0; i < act_result_num; ++i) {
            std::stringstream buf;
            buf << "result_" << i;
            ar & bs::make_nvp<PriceList>(buf.str().c_str(), *m_pBuffer[i]);
        }        
    }

    template<class Archive>
    void load(Archive & ar, const unsigned int version) {
        namespace bs = boost::serialization;
        ar & BOOST_SERIALIZATION_NVP(m_name);
        ar & BOOST_SERIALIZATION_NVP(m_params);
        ar & BOOST_SERIALIZATION_NVP(m_discard);
        ar & BOOST_SERIALIZATION_NVP(m_result_num);
        ar & BOOST_SERIALIZATION_NVP(m_need_calculate);
        ar & BOOST_SERIALIZATION_NVP(m_optype);
        ar & BOOST_SERIALIZATION_NVP(m_left);
        ar & BOOST_SERIALIZATION_NVP(m_right);
        ar & BOOST_SERIALIZATION_NVP(m_three);
        size_t act_result_num = 0;
        ar & BOOST_SERIALIZATION_NVP(act_result_num);
        for (size_t i = 0; i < act_result_num; ++i) {
            m_pBuffer[i] = new PriceList();
            std::stringstream buf;
            buf << "result_" << i;
            ar & bs::make_nvp<PriceList>(buf.str().c_str(), *m_pBuffer[i]);
        }
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

#if HKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(IndicatorImp)
#endif

#if HKU_SUPPORT_SERIALIZATION
#define INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION private:\
    friend class boost::serialization::access; \
    template<class Archive> \
    void serialize(Archive & ar, const unsigned int version) { \
        ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp); \
    }
#else
#define INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

#define INDICATOR_IMP(classname) public: \
    virtual bool check(); \
    virtual void _calculate(const Indicator& data); \
    virtual IndicatorImpPtr _clone() { return make_shared<classname>(); } 

#define INDICATOR_NEED_CONTEXT public: \
    virtual bool isNeedContext() const { return true; }


typedef shared_ptr<IndicatorImp> IndicatorImpPtr;

HKU_API std::ostream & operator<<(std::ostream&, const IndicatorImp&);
HKU_API std::ostream & operator<<(std::ostream&, const IndicatorImpPtr&);

inline size_t IndicatorImp::getResultNumber() const {
    return m_result_num;
}

inline size_t IndicatorImp::discard() const {
    return m_discard;
}

inline size_t IndicatorImp::size() const {
    return m_pBuffer[0] ? m_pBuffer[0]->size() : 0;
}

inline string IndicatorImp::name() const { 
    return m_name; 
}

inline void IndicatorImp::name(const string& name) {
    m_name = name; 
}

inline bool IndicatorImp::isLeaf() const { 
    return m_optype == LEAF ? true : false; 
}

inline KData IndicatorImp::getContext() const { 
    return getParam<KData>("kdata"); 
}

inline price_t IndicatorImp::get(size_t pos, size_t num) {
#if CHECK_ACCESS_BOUND
    if ((m_pBuffer[num] == NULL) || pos>= m_pBuffer[num]->size()) {
        throw(std::out_of_range("Try to access value out of bounds! "
                + name() + " [IndicatorImp::get]"));
        return Null<price_t>();
    }
#endif
    return (*m_pBuffer[num])[pos];
}

inline void IndicatorImp::_set(price_t val, size_t pos, size_t num) {
#if CHECK_ACCESS_BOUND
    if ((m_pBuffer[num] == NULL) || pos>= m_pBuffer[num]->size()) {
        throw(std::out_of_range("Try to access value out of bounds! "
                + name() + " [IndicatorImp::_set]"));
    }
    (*m_pBuffer[num])[pos] = val;
#else
    (*m_pBuffer[num])[pos] = val;
#endif
}

} /* namespace hku */
#endif /* INDICATORIMP_H_ */
