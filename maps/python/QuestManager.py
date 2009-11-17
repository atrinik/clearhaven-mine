## @file
## This file has the QuestManager class, which is used by all Python
## quests to provide common quest-related functions.

from Atrinik import *
import string, os

## The Quest Manager class.
class QuestManager:
	## @defgroup quest_types Quest types
	## All the different quest types this class handles.
	## @{

	## Kill X monsters type quest.
	QUEST_TYPE_KILL = 1
	## Get item X by killing monster Y type quest.
	QUEST_TYPE_KILL_ITEM = 2
	## @}

	## Marks the quest as completed.
	QUEST_STATUS_COMPLETED = 1

	## The quest object inside player's quest container.
	quest_object = None
	## Information about the quest.
	quest = {}
	## Activator object.
	activator = None

	## Initialize function.
	## @param activator The activator object.
	## @param quest Information about the quest as a dictionary.
	def __init__(self, activator, quest):
		self.quest_object = activator.GetQuestObject(quest["quest_name"])
		self.quest = quest
		self.activator = activator

	## Start a quest, calling the relevant function in Atrinik Python
	## plugin, and setting any initial values for different quest types.
	def start(self):
		self.quest_object = self.activator.StartQuest(self.quest["quest_name"])
		self.quest_object.sub_type_1 = self.quest["type"]

		# For the kill type quest, set the last_grace field to the value
		# of monsters we have to kill.
		if self.quest["type"] == self.QUEST_TYPE_KILL:
			self.quest_object.last_grace = self.quest["kills"]

	## Check if the quest has been started.
	def started(self):
		return self.quest_object != None

	## Check if the quest has been finished by checking the relevant
	## values for different quest types.
	def finished(self):
		# For the kill type quest check if we have killed enough
		# monsters.
		if self.quest["type"] == self.QUEST_TYPE_KILL:
			if self.quest_object.last_sp >= self.quest_object.last_grace:
				return 1
		# For the kill item quest type check for the quest item.
		elif self.quest["type"] == self.QUEST_TYPE_KILL_ITEM:
			quest_item = self.activator.CheckInventory(1, self.quest["arch_name"], self.quest["item_name"])

			if quest_item and quest_item.f_quest_item:
				return 1

		return 0

	## Complete a quest.
	def complete(self):
		self.quest_object.magic = self.QUEST_STATUS_COMPLETED

		# Do anything extra for the kill item quest type.
		if self.quest["type"] == self.QUEST_TYPE_KILL_ITEM:
			# Find the quest item that is being looked for.
			quest_item = self.activator.CheckInventory(1, self.quest["arch_name"], self.quest["item_name"])

			# If we're keeping the item, we will want to adjust some
			# flags.
			if "quest_item_keep" in self.quest and self.quest["quest_item_keep"]:
				quest_item.f_quest_item = 0
				quest_item.f_startequip = 0
			# Otherwise remove it.
			else:
				quest_item.Remove()

	## Check if a quest has been completed before.
	def completed(self):
		# Check the quest object's magic field to see if the quest has
		# been completed.
		if self.quest_object.magic == self.QUEST_STATUS_COMPLETED:
			return 1

		return 0

	## Get number of monsters the player needs to kill to complete the
	## quest.
	def num_to_kill(self):
		if not self.quest_object or self.quest["type"] != self.QUEST_TYPE_KILL:
			return -1

		return self.quest_object.last_grace - self.quest_object.last_sp