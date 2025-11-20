/**
 * @file input_module_base.h
 * @author HUANG He (he.hu4ng@outlook.com)
 * @brief Base class of audio mixer input modules
 * @version 0.1
 * @date 2025-10-12
 *
 * @copyright Copyright (c) 2025
 *
 */
#pragma once

#include <memory>

#include "audio_prop_def.h"
#include "audio_queue.h"

/**
 * @brief Audio mixer input module base class
 */
template<audio_sample_type AudioType>
class input_module_base
{
	using QueueType = audio_queue<AudioType>;

	std::shared_ptr<QueueType> m_target_queue;
	audio_context			   m_output_context;
	bool					   m_is_active;

public:

	input_module_base()
		: m_is_active(false)
	{}

	input_module_base(std::shared_ptr<QueueType> sp_target, const audio_context& expected_ctx)
		: m_target_queue(sp_target),
		  m_is_active(false),
		  m_output_context(expected_ctx)
	{}

	virtual ~input_module_base() = default;

	// You don't want to copy of move a audio mixer input module.
	input_module_base(const input_module_base& other)			 = delete;
	input_module_base(input_module_base&& other)				 = delete;
	input_module_base& operator=(const input_module_base& other) = delete;
	input_module_base& operator=(input_module_base&& other)		 = delete;

	// Pure virtual start and stop mmodule fucntion
	virtual void start() = 0;
	virtual void stop()	 = 0;

	/**
	 * @brief See if the module is currently active
	 * 
	 * @return true module is active
	 * @return false module is not active
	 */
	[[nodiscard]] bool active() const { return m_is_active; }
};

/* Input module type concept */
template<typename ClassType, typename AudioType>
concept InputModuleType =
	requires(ClassType class_obj) {
		requires audio_sample_type<AudioType>;
		requires std::derived_from<ClassType, input_module_base<AudioType>>;
	};