#!/usr/bin/python
import pexpect
import os


TEMP_FILE = "temp_test_authenticate.auth"
PROMPT = ">>"

MAX_USER_LEN = 80
MAX_PASS_LEN = 80

TEST_SCENARIOS = (
		#happy-flow
		("mitsi", "1"),

		#zero length username/password
		("", "no_user"),
		("no_pass", ""),

		#zero length everything
		("", ""),

		# LONG username,
		("a" * MAX_USER_LEN, "long_username"), 
		# LONG password
		("long_pass", "a" * MAX_PASS_LEN), 

		# LONG username & password
		("b" * MAX_PASS_LEN, "b" * MAX_PASS_LEN), 

		#duplicate user (both should just work, weird spec)
		("duplicate_user", "dup"),
		("duplicate_user", "dup2"),
		)

def cleanFile():
	try:
		os.remove(TEMP_FILE)
	except:
		pass

class CreateAuthentication:
	def __init__(self, cmd, algoName):
		self.__session = pexpect.spawn("%s %s %s" %(cmd, algoName, TEMP_FILE))
		try:
			self.__session.expect(PROMPT)
			if self.__session.before != "":
				raise Exception()
		except:
			raise Exception("Error from cmd. Error: %s" %self.__session.before)

	
	
	def addUser(self, user, password):
		self.__session.sendline("%s\t%s" %(user, password))
		self.__session.expect(PROMPT)

	def quit(self):
		#consume any pending data
		try:
			self.__session.expect(PROMPT, timeout=1)
		except:
			pass

		self.__session.sendline("quit")

		if "" != self.__session.before:
			return False

		return True

	def eofQuit(self):
		#consume any pending data
		try:
			self.__session.expect(PROMPT, timeout=1)
		except:
			pass

		self.__session.sendeof()
		if "" != self.__session.before:
			return False

		return True

class Authenticate:
	def __init__(self, cmd):
		self.__session = pexpect.spawn("%s %s" %(cmd, TEMP_FILE))
		try:
			self.__session.expect(PROMPT)
			if self.__session.before != "":
				raise Exception()
		except:
			raise Exception("Error from cmd. Error: %s" %self.__session.before)
	
	
	def authenticate(self, user, password):
		self.__session.sendline("%s\t%s" %(user, password))
		self.__session.expect(PROMPT)
		if -1 != self.__session.before.find("Approved."):
			return True
		if -1 != self.__session.before.find("Denied."):
			return False

		raise Exception("Error from cmd. Error: %s" %self.__session.before)

	def quit(self):
		#consume any pending data
		try:
			self.__session.expect(PROMPT, timeout=1)
		except:
			pass

		self.__session.sendline("quit")

		if "" != self.__session.before:
			return False

		return True

	def eofQuit(self):
		#consume any pending data
		try:
			self.__session.expect(PROMPT, timeout=1)
		except:
			pass

		self.__session.sendeof()
		if "" != self.__session.before:
			return False

		return True


def testAuthentication(cmd):
	ret = True

	cleanFile()
	a = CreateAuthentication(cmd[0], "MD5")
	for (username, password) in TEST_SCENARIOS:
		a.addUser(username, password)
	a.quit()

	#first, try authenticating properly
	a = Authenticate(cmd[1])
	for (username, password) in TEST_SCENARIOS:
		if True != a.authenticate(username, password):
			print "ERROR: failed for (%s : %s)" %(username, password)
	
	#now, mess with the usernames
	a = Authenticate(cmd[1])
	for (username, password) in TEST_SCENARIOS:
		username = ("bad_user" + username)[:MAX_USER_LEN]
		if False != a.authenticate(username, password):
			print "ERROR: allowed invalid user to authenticate! for (%s : %s)" %(username, password)
			ret = False

	a = Authenticate(cmd[1])
	for (username, password) in TEST_SCENARIOS:
		password = ("bad_pass" + password)[:MAX_PASS_LEN]
		if False != a.authenticate(username, password):
			print "ERROR: allowed invalid user to authenticate! for (%s : %s)" %(username, password)
			ret = False

	return ret


print "simple authenticate: "
print testAuthentication(("./create_authentication", "./authenticate"))

print "\nsalty authenticate: "
print testAuthentication(("./create_salty_authentication", "./salty_authenticate"))
	

	
