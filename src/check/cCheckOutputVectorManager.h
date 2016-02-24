#ifndef CCHECKOUTPUTVECTORMANAGER_H
#define CCHECKOUTPUTVECTORMANAGER_H

#include <omnetpp.h>
#include <limits>

//Std
#include <vector>

class cCheckOutputVectorManager : public cOutputVectorManager
{
    protected:
        class Constraint
        {
                friend class cCheckOutputVectorManager;
            private:
                enum Type
                {
                    min, max, avg_min, avg_max, interval_min, interval_max, sum_max
                } type;
                double value;
                bool violation;
                simtime_t intervallStart;
                std::vector<std::pair<simtime_t, simtime_t>> intervals;

                Constraint(Type t) :
                        type(t), value(0), violation(false), intervallStart(0)
                {
                }
        };
        class IntervalConstraint : public Constraint
        {
                friend class cCheckOutputVectorManager;
            private:
                simtime_t last;

                IntervalConstraint(Type t) :
                        Constraint(t), last(0)
                {
                }
        };
        class SumConstraint : public Constraint
        {
                friend class cCheckOutputVectorManager;
            private:
                double sum;

                SumConstraint(Type t) :
                        Constraint(t), sum(0)
                {
                }
        };
        class AverageConstraint : public Constraint
        {
                friend class cCheckOutputVectorManager;
            private:
                size_t noSamples;
                size_t requiredSamples;
                double *samples;
                size_t samplesPos;

                AverageConstraint(Type t, size_t rs) :
                        Constraint(t), noSamples(0), requiredSamples(rs), samplesPos(0)
                {
                    samples = new double[rs];
                }
                ~AverageConstraint()
                {
                    delete samples;
                }
        };
        struct sVectorData
        {

                std::vector<Constraint*> constraints;
                std::string moduleName;
                std::string vectorName;
        };
        std::vector<sVectorData*> vectordata;
        cXMLElement * xmlConfiguration;
        bool endSimulation;
        bool report;
        std::string reportFilename;
    public:
        /**
         * Constructor.
         */
        cCheckOutputVectorManager();

        /**
         * Destructor.
         */
        virtual ~cCheckOutputVectorManager();

        /**
         * Opens collecting. Called at the beginning of a simulation run.
         */
        virtual void startRun() override;

        /**
         * Closes collecting. Called at the end of a simulation run.
         */
        virtual void endRun() override;

        /**
         * Registers a vector and returns a handle.
         */
        virtual void *registerVector(const char *modulename, const char *vectorname) override;

        /**
         * Deregisters the output vector.
         */
        virtual void deregisterVector(void *vectorhandle) override;

        /**
         * Sets an attribute of an output vector.
         */
        virtual void setVectorAttribute(void *vectorhandle, const char *name, const char *value) override;

        /**
         * Writes the (time, value) pair into the output file.
         */
        virtual bool record(void *vectorhandle, simtime_t t, double value) override;

        /**
         * Returns nullptr, because this class doesn't use a file.
         */
        const char *getFileName() const override;

        /**
         * Performs a database commit.
         */
        virtual void flush() override;

    private:
        void outputReport();
};

#endif

