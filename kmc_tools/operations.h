/*
  This file is a part of KMC software distributed under GNU GPL 3 licence.
  The homepage of the KMC project is http://sun.aei.polsl.pl/kmc
  
  Authors: Marek Kokot
  
  Version: 3.1.0
  Date   : 2018-05-10
*/

#ifndef _OPERATIONS_H
#define _OPERATIONS_H


#ifdef ENABLE_DEBUG
#include "config.h"
#endif

#ifdef ENABLE_LOGGER
#include "develop.h"
#endif

#include <iostream>
#include <vector>
#include "bundle.h"

//************************************************************************************************************
// C2ArgOper - abstract class representing 2 argument's operation
//************************************************************************************************************
template<unsigned SIZE> class C2ArgOper : public CInput<SIZE>
{
protected:	
	CBundle<SIZE>* input1, *input2;
	CounterOpType counter_op_type;
public:
	C2ArgOper(CBundle<SIZE>* input1, CBundle<SIZE>* input2, CounterOpType counter_op_type) :
		input1(input1), input2(input2), counter_op_type(counter_op_type)
	{
	}
	void EqualsToOuputBundle(CBundle<SIZE>& output_bundle)
	{
		switch (counter_op_type)
		{
		case CounterOpType::MIN:
			output_bundle.Insert(input1->TopKmer(), MIN(input1->TopCounter(), input2->TopCounter()));
			break;
		case CounterOpType::MAX:
			output_bundle.Insert(input1->TopKmer(), MAX(input1->TopCounter(), input2->TopCounter()));
			break;
		case CounterOpType::SUM:
			output_bundle.Insert(input1->TopKmer(), input1->TopCounter() + input2->TopCounter());
			break;
		case CounterOpType::DIFF:
			if (input1->TopCounter() > input2->TopCounter())
				output_bundle.Insert(input1->TopKmer(), input1->TopCounter() - input2->TopCounter());
			break;
		case CounterOpType::FROM_DB1:
			output_bundle.Insert(input1->TopKmer(), input1->TopCounter());
			break;
		case CounterOpType::FROM_DB2:
			output_bundle.Insert(input1->TopKmer(), input2->TopCounter());
			break;
		case CounterOpType::NONE:
			break;
		default:
			break;
		}
	}
	void IgnoreRest() override
	{		
		input1->IgnoreRest();
		input2->IgnoreRest();
	}

	~C2ArgOper() override
	{
		delete input1;
		delete input2;
	}
};

//************************************************************************************************************
// CUnion - implementation of union operation on 2 k-mer's sets.
//************************************************************************************************************
template <unsigned SIZE> class CUnion : public C2ArgOper<SIZE>
{		
public:
	CUnion(CBundle<SIZE>* input1, CBundle<SIZE>* input2, CounterOpType counter_op_type) : C2ArgOper<SIZE>(input1, input2, counter_op_type)
	{
	}
	void NextBundle(CBundle<SIZE>& bundle) override
	{
		while (!this->input1->Finished() && !this->input2->Finished())
		{
			if (bundle.Full())
			{
				return;
			}	
			if (this->input1->TopKmer() == this->input2->TopKmer())
			{
				this->EqualsToOuputBundle(bundle);
				this->input1->Pop();
				this->input2->Pop();
			}
			else if (this->input1->TopKmer() < this->input2->TopKmer())
			{
				bundle.Insert(this->input1->TopKmer(), this->input1->TopCounter());
				this->input1->Pop();
			}
			else
			{
				bundle.Insert(this->input2->TopKmer(), this->input2->TopCounter());
				this->input2->Pop();
			}
		}
		CBundle<SIZE>* non_empty_bundle = this->input1->Finished() ? this->input2 : this->input1;
		while (!non_empty_bundle->Finished())
		{
			if (bundle.Full())
			{
				return;
			}
			bundle.Insert(non_empty_bundle->TopKmer(), non_empty_bundle->TopCounter());
			non_empty_bundle->Pop();
		}
		this->finished = true;
	}
};

//************************************************************************************************************
// CIntersection - implementation of intersection operation on 2 k-mer's sets.
//************************************************************************************************************
template<unsigned SIZE> class CIntersection : public C2ArgOper<SIZE>
{		
public:
	CIntersection(CBundle<SIZE>* input1, CBundle<SIZE>* input2, CounterOpType counter_op_type) : C2ArgOper<SIZE>(input1, input2, counter_op_type)
	{
	}
	void NextBundle(CBundle<SIZE>& bundle) override
	{
		while (!this->input1->Finished() && !this->input2->Finished())
		{
			if (this->input1->TopKmer() == this->input2->TopKmer())
			{
				this->EqualsToOuputBundle(bundle);
				this->input1->Pop();
				this->input2->Pop();
				if (bundle.Full())
					return;
			}
			else if (this->input1->TopKmer() < this->input2->TopKmer())
				this->input1->Pop();
			else
				this->input2->Pop();
		}
		if (!this->input1->Finished())
			this->input1->IgnoreRest();
		if (!this->input2->Finished())
			this->input2->IgnoreRest();
		this->finished = true;
	}
};

//************************************************************************************************************
// CKmersSubtract - implementation of subtraction operation of 2 k-mer's sets.
// If k-mer exists in both input it is absent in result (counters does not matter).
//************************************************************************************************************
template<unsigned SIZE> class CKmersSubtract : public C2ArgOper<SIZE>
{	
public:
	CKmersSubtract(CBundle<SIZE>* input1, CBundle<SIZE>* input2) : C2ArgOper<SIZE>(input1, input2, CounterOpType::NONE)
	{
	}
	void NextBundle(CBundle<SIZE>& bundle) override
	{
		while (!this->input1->Finished() && !this->input2->Finished())
		{
			if (this->input2->TopKmer() < this->input1->TopKmer())
				this->input2->Pop();
			else if (this->input2->TopKmer() == this->input1->TopKmer())
			{
				this->input1->Pop();
				this->input2->Pop();
			}
			else
			{
				bundle.Insert(this->input1->TopKmer(), this->input1->TopCounter());
				this->input1->Pop();
				if (bundle.Full())
					return;
			}
		}
		
		if(!this->input2->Finished())
			this->input2->IgnoreRest(); 

		while (!this->input1->Finished())
		{
			if (bundle.Full())
				return;
			bundle.Insert(this->input1->TopKmer(), this->input1->TopCounter());
			this->input1->Pop();
		}
		this->finished = true;
	}
};




//************************************************************************************************************
// CCountersSubtract - implementation of subtraction operation of 2 k-mer's sets.
// If k-mer exists in both input their counters are subtracted.
//************************************************************************************************************
template<unsigned SIZE> class CCountersSubtract : public C2ArgOper<SIZE>
{
public:
	CCountersSubtract(CBundle<SIZE>* input1, CBundle<SIZE>* input2, CounterOpType counter_op_type) : C2ArgOper<SIZE>(input1, input2, counter_op_type)
	{
	}
	void NextBundle(CBundle<SIZE>& bundle) override
	{
		while (!this->input1->Finished() && !this->input2->Finished())
		{
			if (this->input2->TopKmer() < this->input1->TopKmer())
				this->input2->Pop();
			else if (this->input2->TopKmer() == this->input1->TopKmer())
			{
				this->EqualsToOuputBundle(bundle);
				this->input1->Pop();
				this->input2->Pop();
				if (bundle.Full())
					return;
			}
			else
			{
				bundle.Insert(this->input1->TopKmer(), this->input1->TopCounter());
				this->input1->Pop();
				if (bundle.Full())
					return;
			}
		}

		if (!this->input2->Finished())
			this->input2->IgnoreRest();

		while (!this->input1->Finished())
		{
			if (bundle.Full())
				return;			
			bundle.Insert(this->input1->TopKmer(), this->input1->TopCounter());
			this->input1->Pop();
		}
		this->finished = true;
	}
};

template<unsigned SIZE> class CComparer 
{
	CBundle<SIZE>* input1, *input2;
public:
	CComparer(CBundle<SIZE>* input1, CBundle<SIZE>* input2) : input1(input1), input2(input2)
	{
	}

	bool Equals()
	{		
		while (!this->input1->Finished() && !this->input2->Finished())
		{
			if (this->input1->TopCounter() != this->input2->TopCounter())
			{
				this->input1->IgnoreRest();
				this->input2->IgnoreRest();
				return false;
			}
			if (!(this->input1->TopKmer() == this->input2->TopKmer()))
			{
				this->input1->IgnoreRest();
				this->input2->IgnoreRest();
				return false;
			}

			this->input1->Pop();
			this->input2->Pop();
		}
		if (!this->input1->Finished() || !this->input2->Finished())
		{
			std::cout << "one of input is not finished\n";
			this->input1->IgnoreRest();
			this->input2->IgnoreRest();
			return false;
		}
		return true;
	}
};

template<unsigned SIZE>
class CAnalyzeOperation
{
	CBundle<SIZE> *input1, *input2;
	uint64_t* occ_cnts;	
	uint32_t max_cov;
	uint32_t max_cov_asm;	
	uint64_t shared_kmer_counter;
	std::string output_fn;	
		
	void process_impl()
	{
		uint32 get1 = 0;
		uint32 get2 = 0;

		CKmer<SIZE> kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
		uint32 counter2 = this->input2->data.kmers_with_counters[get2].counter;
		CKmer<SIZE> kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
		uint32 counter1 = this->input1->data.kmers_with_counters[get1].counter;


		uint32 left1 = this->input1->NRecLeft();
		uint32 left2 = this->input2->NRecLeft();


		while (true)
		{
			if (kmer1 == kmer2)
			{

				uint32_t idx = counter1 >= max_cov ?  max_cov * (max_cov_asm + 1): counter1 * (max_cov_asm + 1);
				uint32_t idx_rmd = counter2 >=  max_cov_asm? max_cov_asm : counter2; 
				occ_cnts[idx + idx_rmd] += 1;
				++shared_kmer_counter;
	
				++get1;
				++get2;

				if (--left1)
				{

					kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
					counter1 = this->input1->data.kmers_with_counters[get1].counter;

				}
				else
				{
					this->input1->data.get_pos = get1;
					if (!this->input1->Finished())
					{
						get1 = 0;
						kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
						counter1 = this->input1->data.kmers_with_counters[get1].counter;
						left1 = this->input1->NRecLeft();
					}
					else
						break;
				}

				if (--left2)
				{
					kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
					counter2 = this->input2->data.kmers_with_counters[get2].counter;
				}
				else
				{
					this->input2->data.get_pos = get2;
					if (!this->input2->Finished())
					{
						get2 = 0;
						kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
						counter2 = this->input2->data.kmers_with_counters[get2].counter;
						left2 = this->input2->NRecLeft();
						
					}
					else
						break;
				}
			}
			else if (kmer1 < kmer2)
			{

				uint32_t idx = counter1 >= max_cov ?  (max_cov)* (max_cov_asm + 1): (counter1) * (max_cov_asm + 1);
				uint32_t idx_rmd = 0; 
				occ_cnts[idx + idx_rmd] += 1;
				++get1;
				if (--left1)
				{
					kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
					counter1 = this->input1->data.kmers_with_counters[get1].counter;
				}
				else
				{
					this->input1->data.get_pos = get1;
					if (!this->input1->Finished())
					{
						get1 = 0;
						kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
						counter1 = this->input1->data.kmers_with_counters[get1].counter;
						left1 = this->input1->NRecLeft();
					}
					else
						break;
				}
			}
			else
			{
				uint32_t idx = 0;
				uint32_t idx_rmd = counter2 >=  max_cov_asm? max_cov_asm : counter2 ; 
				occ_cnts[idx + idx_rmd] += 1;
				++get2;
				if (--left2)
				{
					kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
					counter2 = this->input2->data.kmers_with_counters[get2].counter;
				}
				else
				{
					this->input2->data.get_pos = get2;
					if (!this->input2->Finished())
					{
						get2 = 0;
						kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
						counter2 = this->input2->data.kmers_with_counters[get2].counter;
						left2 = this->input2->NRecLeft();

					}
					else
						break;
				}
			}
		}
		this->input1->data.get_pos = get1;
		this->input2->data.get_pos = get2;
	}
	public:
		CAnalyzeOperation(CBundle<SIZE> *_input1, CBundle<SIZE> *_input2, uint32_t _mc, uint32_t _mca, std::string fn): input1(_input1), input2(_input2), max_cov(_mc), max_cov_asm(_mca), output_fn(fn) {
			occ_cnts = new uint64_t[(max_cov + 1)* (max_cov_asm + 1)];
			memset(occ_cnts, 0, sizeof(uint64_t) * (max_cov+1) * (max_cov_asm+1));
			shared_kmer_counter = 0;	
		}
		void Process()	
		{
			if (!this->input1->Finished() && !this->input2->Finished()) 
				process_impl();
			if (!this->input1->Finished()) {
				CKmer<SIZE> kmer1;
				uint32 counter1;
				while (!this->input1->Finished())
				{
					kmer1 = this->input1->TopKmer();
					counter1 = this->input1->TopCounter();
					uint32_t idx = counter1 >= max_cov ?  max_cov * ( 1 + max_cov_asm): (counter1) * (max_cov_asm + 1);
					uint32_t idx_rmd = 0; 
					occ_cnts[idx + idx_rmd] += 1;
					this->input1->Pop();
				}
			}	
			if (!this->input2->Finished()) {
				CKmer<SIZE> kmer2;
				uint32 counter2;
				while (!this->input2->Finished())
				{
					kmer2 = this->input1->TopKmer();
					counter2 = this->input1->TopCounter();
					uint32_t idx = 0;
					uint32_t idx_rmd = counter2 >=  max_cov_asm? max_cov_asm : counter2; 
					occ_cnts[idx + idx_rmd] += 1;
					this->input2->Pop();
				}
			}
			FILE *fp = fopen(output_fn.c_str(), "w");
			for ( int i = 0; i <= max_cov; ++i) {
				uint32_t idx = i * (max_cov_asm + 1);
				fprintf(fp, "%d\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\t%lu\n", i, occ_cnts[idx], occ_cnts[idx+1], occ_cnts[idx+2], occ_cnts[idx+3], occ_cnts[idx+4], occ_cnts[idx+5], occ_cnts[idx+6]);
				//std::cout << i;
				//for (int j = 0; j <= max_cov_asm; ++j) 
					//std::cout << '\t' << occ_cnts[idx + j];
				//std::cout << '\n';	
			}	
			fclose(fp);
			std::cerr <<"shared k-mers:" << shared_kmer_counter << std::endl;
		}	
		~CAnalyzeOperation() {
			free(occ_cnts);
		}

};


template<unsigned SIZE>
class CSimpleOperation
{
	CBundle<SIZE>* input1, *input2;
	std::vector<COutputBundle<SIZE>*>& outputs;

	std::vector<COutputBundle<SIZE>*> outputs_for_equals;
	std::vector<COutputBundle<SIZE>*> outputs_for_1st_lower;
	std::vector<COutputBundle<SIZE>*> outputs_for_2nd_lower;


public:
	CSimpleOperation(CBundle<SIZE>* input1, CBundle<SIZE>* input2, std::vector<COutputBundle<SIZE>*>& outputs) :
		input1(input1),
		input2(input2),
		outputs(outputs)
	{
		for (auto o : outputs)
		{
			auto op = o->GetOpType();
			if (op == CSimpleOutputDesc::OpType::INTERSECT || op == CSimpleOutputDesc::OpType::UNION || op == CSimpleOutputDesc::OpType::COUNTERS_SUBTRACTION || op == CSimpleOutputDesc::OpType::REVERSE_COUNTERS_SUBTRACTION)
				outputs_for_equals.push_back(o);
			if (op == CSimpleOutputDesc::OpType::UNION || op == CSimpleOutputDesc::OpType::KMERS_SUBTRACTION || op == CSimpleOutputDesc::OpType::COUNTERS_SUBTRACTION)
				outputs_for_1st_lower.push_back(o);
			if (op == CSimpleOutputDesc::OpType::UNION || op == CSimpleOutputDesc::OpType::REVERSE_KMERS_SUBTRACTION || op == CSimpleOutputDesc::OpType::REVERSE_COUNTERS_SUBTRACTION)
				outputs_for_2nd_lower.push_back(o);
			else
			{
				//should never be here
			}
		}
	}

	struct CEqNotifier
	{
		static void Notify(CSimpleOperation<SIZE>& operation, CKmer<SIZE>& kmer, uint32 counter1, uint32 counter2)
		{
			for (auto output : operation.outputs_for_equals)
			{
				uint32 c = 0;
				if (output->GetOpType() == CSimpleOutputDesc::OpType::REVERSE_COUNTERS_SUBTRACTION)
					c = output->GetCounter(counter2, counter1);
				else
					c = output->GetCounter(counter1, counter2);
				output->InsertAndSendIfFull(kmer, c);
			}
		}
	};
	struct CEqNotifierEmpty
	{
		static void Notify(CSimpleOperation<SIZE>& /*operation*/, CKmer<SIZE>& /*kmer*/, uint32 /*counter1*/, uint32 /*counter2*/){}
	};

	struct C1stLowerNotifier
	{
		static void Notify(CSimpleOperation<SIZE>& operation, CKmer<SIZE>& kmer, uint32 counter)
		{
			for (auto output : operation.outputs_for_1st_lower)
				output->InsertAndSendIfFull(kmer, counter);
		}
	};
	struct CLowerNotifierEmpty
	{
		static void Notify(CSimpleOperation<SIZE>& /*operation*/, CKmer<SIZE>& /*kmer*/, uint32 /*counter*/){}
	};

	struct C2ndLowerNotifier
	{
		static void Notify(CSimpleOperation<SIZE>& operation, CKmer<SIZE>& kmer, uint32 counter)
		{
			for (auto output : operation.outputs_for_2nd_lower)
				output->InsertAndSendIfFull(kmer, counter);
		}
	};
	
	template<typename EQ_NOTIFIER, typename FIRST_LOWER_NOTIFIER, typename SECOND_LOWER_NOTIFIER>
	void process_impl()
	{
		uint32 get1 = 0;
		uint32 get2 = 0;

		CKmer<SIZE> kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
		uint32 counter2 = this->input2->data.kmers_with_counters[get2].counter;
		CKmer<SIZE> kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
		uint32 counter1 = this->input1->data.kmers_with_counters[get1].counter;


		uint32 left1 = this->input1->NRecLeft();
		uint32 left2 = this->input2->NRecLeft();


		while (true)
		{
			if (kmer1 == kmer2)
			{
				EQ_NOTIFIER::Notify(*this, kmer1, counter1, counter2);

	
				++get1;
				++get2;

				if (--left1)
				{

					kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
					counter1 = this->input1->data.kmers_with_counters[get1].counter;

				}
				else
				{
					this->input1->data.get_pos = get1;
					if (!this->input1->Finished())
					{
						get1 = 0;
						kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
						counter1 = this->input1->data.kmers_with_counters[get1].counter;
						left1 = this->input1->NRecLeft();
					}
					else
						break;
				}

				if (--left2)
				{
					kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
					counter2 = this->input2->data.kmers_with_counters[get2].counter;
				}
				else
				{
					this->input2->data.get_pos = get2;
					if (!this->input2->Finished())
					{
						get2 = 0;
						kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
						counter2 = this->input2->data.kmers_with_counters[get2].counter;
						left2 = this->input2->NRecLeft();
						
					}
					else
						break;
				}
			}
			else if (kmer1 < kmer2)
			{

				FIRST_LOWER_NOTIFIER::Notify(*this, kmer1, counter1);
				++get1;
				if (--left1)
				{
					kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
					counter1 = this->input1->data.kmers_with_counters[get1].counter;
				}
				else
				{
					this->input1->data.get_pos = get1;
					if (!this->input1->Finished())
					{
						get1 = 0;
						kmer1 = this->input1->data.kmers_with_counters[get1].kmer;
						counter1 = this->input1->data.kmers_with_counters[get1].counter;
						left1 = this->input1->NRecLeft();
					}
					else
						break;
				}
			}
			else
			{
				SECOND_LOWER_NOTIFIER::Notify(*this, kmer2, counter2);

				++get2;
				if (--left2)
				{
					kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
					counter2 = this->input2->data.kmers_with_counters[get2].counter;
				}
				else
				{
					this->input2->data.get_pos = get2;
					if (!this->input2->Finished())
					{
						get2 = 0;
						kmer2 = this->input2->data.kmers_with_counters[get2].kmer;
						counter2 = this->input2->data.kmers_with_counters[get2].counter;
						left2 = this->input2->NRecLeft();

					}
					else
						break;
				}
			}
		}
		this->input1->data.get_pos = get1;
		this->input2->data.get_pos = get2;
	}

	void Process()
	{

#ifdef ENABLE_LOGGER
		CTimer timer;
		timer.start();
#endif
		bool notify_equal = outputs_for_equals.size() != 0;
		bool notify_1st_lower = outputs_for_1st_lower.size() != 0;
		bool notify_2nd_lower = outputs_for_2nd_lower.size() != 0;


		if (!this->input1->Finished() && !this->input2->Finished())
		{
			if (!notify_equal && !notify_1st_lower && notify_2nd_lower)
			{
				process_impl<CEqNotifierEmpty, CLowerNotifierEmpty, C2ndLowerNotifier>();
			}
			else if (!notify_equal && notify_1st_lower && !notify_2nd_lower)
			{
				process_impl<CEqNotifierEmpty, C1stLowerNotifier, CLowerNotifierEmpty>();
			}
			else if (!notify_equal && notify_1st_lower && notify_2nd_lower)
			{
				process_impl<CEqNotifierEmpty, C1stLowerNotifier, C2ndLowerNotifier>();
			}
			else if (notify_equal && !notify_1st_lower && !notify_2nd_lower)
			{
				process_impl<CEqNotifier, CLowerNotifierEmpty, CLowerNotifierEmpty>();
			}
			else if (notify_equal && !notify_1st_lower && notify_2nd_lower)
			{
				process_impl<CEqNotifier, CLowerNotifierEmpty, C2ndLowerNotifier>();
			}
			else if (notify_equal && notify_1st_lower && !notify_2nd_lower)
			{
				process_impl<CEqNotifier, C1stLowerNotifier, CLowerNotifierEmpty>();
			}
			else if (notify_equal && notify_1st_lower && notify_2nd_lower)
			{
				process_impl<CEqNotifier, C1stLowerNotifier, C2ndLowerNotifier>();
			}
			else
			{
				//sould never be here
			}
			
		}

		if (notify_1st_lower)
		{
			while (!this->input1->Finished())
			{
				for (auto output : outputs_for_1st_lower)
					output->InsertAndSendIfFull(this->input1->TopKmer(), this->input1->TopCounter());
				this->input1->Pop();
			}
		}
		else
			this->input1->IgnoreRest();

		if (notify_2nd_lower)
		{
			while (!this->input2->Finished())
			{
				for (auto output : outputs_for_2nd_lower)
					output->InsertAndSendIfFull(this->input2->TopKmer(), this->input2->TopCounter());
				this->input2->Pop();
			}
		}
		else
			this->input2->IgnoreRest();

		for (auto output : outputs)
			output->NotifyFinish();


#ifdef ENABLE_LOGGER				
		CLoger::GetLogger().log_operation("Process", this, timer.get_time());
#endif
	}
};


#endif

// ***** EOF
