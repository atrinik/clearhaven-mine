import Atrinik
import string

activator = Atrinik.WhoIsActivator()
me = Atrinik.WhoAmI()

msg = Atrinik.WhatIsMessage().strip().lower()

if activator.DoKnowSkill(Atrinik.GetSkillNr("impact weapons")) == 1 or activator.DoKnowSkill(Atrinik.GetSkillNr("slash weapons")) == 1 or activator.DoKnowSkill(Atrinik.GetSkillNr("cleave weapons")) == 1 or activator.DoKnowSkill(Atrinik.GetSkillNr("pierce weapons")) == 1:
	me.SayTo(activator, "You already know a weapon skill.")

elif msg == "slash":
	skill = Atrinik.GetSkillNr("slash weapons")

	if skill == -1:
		me.SayTo(activator, "Unknown skill.")
	else:
		if activator.DoKnowSkill(skill) == 1:
			me.SayTo(activator, "You already know this skill.")
		else:
			activator.Write("%s gives you a sword." % me.name, 0)
			activator.AcquireSkill(skill, Atrinik.LEARN)
			activator.Apply(activator.CreateObjectInside("shortsword", 1, 1, 1), 0)

elif msg == "impact":
	skill = Atrinik.GetSkillNr("impact weapons")

	if skill == -1:
		me.SayTo(activator, "Unknown skill.")
	else:
		if activator.DoKnowSkill(skill) == 1:
			me.SayTo(activator, "You already know this skill.")
		else:
			activator.Write("%s gives you a small morningstar." % me.name, 0)
			activator.AcquireSkill(skill, 0)
			activator.Apply(activator.CreateObjectInside("mstar_small", 1, 1, 1), 0)

elif msg == "cleave":
	skill = Atrinik.GetSkillNr("cleave weapons")

	if skill == -1:
		me.SayTo(activator, "Unknown skill.")
	else:
		if activator.DoKnowSkill(skill) == 1:
			me.SayTo(activator, "You already know this skill.")
		else:
			activator.Write("%s gives you an axe." % me.name, 0)
			activator.AcquireSkill(skill, 0)
			activator.Apply(activator.CreateObjectInside("axe_small", 1, 1, 1), 0)

elif msg == "pierce":
	skill = Atrinik.GetSkillNr("pierce weapons")

	if skill == -1:
		me.SayTo(activator, "Unknown skill.")
	else:
		if activator.DoKnowSkill(skill) == 1:
			me.SayTo(activator, "You already know this skill.")
		else:
			activator.Write("%s gives you a large dagger." % me.name, 0)
			activator.AcquireSkill(skill, 0)
			activator.Apply(activator.CreateObjectInside("dagger_large", 1, 1, 1), 0)

elif msg == "weapons":
	me.SayTo(activator, "\nWe have 4 different weapon skills.\nEach skill allows the use of a special kind of weapons.\nSlash weapons are swords.\nCleave weapons are axe-like weapons.\nPierce weapons are rapiers and daggers.\nImpact weapons are maces or hammers.\nNow select one and tell me: ^slash^, ^cleave^, ^pierce^ or ^impact^?")

elif msg == "hello" or msg == "hi" or msg == "hey":
	me.SayTo(activator, "\nHello! I am %s.\nI will give you your starting weapon skill and your first weapon. Now, tell me which weapon skill do you want.\nYou can select between slash weapons, cleave weapons, pierce weapons or impact weapons.\nAsk me about ^weapons^ to learn more." % me.name)

else:
	activator.Write("%s seems not to notice you.\nYou should try ^hello^, ^hi^ or ^hey^..." % me.name, 0)