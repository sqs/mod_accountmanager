import unittest, urllib2

# define passwds (in .htpasswd)
username_invalid_chars = 'userwith%^invalidchar'
username_too_long = 'username that is too too too too too too too too too ' \
                    'too too too too too too too too too too too long'
passwd = {
    'jsmith': 'abc',
    'alice': 'abc',
}
passwd[username_invalid_chars] = 'abc'
passwd[username_too_long] = 'abc'


class TestModAccountManager(unittest.TestCase):
    def setUp(self):
        self.url = "http://test-mod-accountmanager.mutualauth.org/"
        self.realm = "mod_accountmanager test"

    def test_unauthenticated_request(self):
        res = None
        try:
            res = urllib2.urlopen(self.url)
            self.assertTrue(False, "urlopen should've failed -- requires auth")
        except urllib2.HTTPError as err:
            self.assertEqual(401, err.code)
            self.assertTrue('X-Account-Management-Status' not in err.headers)

    def __authenticated_request(self, user):
        auth_handler = urllib2.HTTPBasicAuthHandler()
        auth_handler.add_password(realm=self.realm, uri=self.url,
                                  user=user, passwd=passwd[user])
        opener = urllib2.build_opener(auth_handler)
        res = opener.open(self.url)
        return res

    def test_authenticated_request(self):
        res = self.__authenticated_request('jsmith')
        self.assertEqual('active; id="jsmith"',
                         res.headers['X-Account-Management-Status'])

    def test_omits_header_if_username_has_invalid_chars(self):
        res = self.__authenticated_request(username_invalid_chars)
        self.assertEqual(200, res.code)
        self.assertTrue('X-Account-Management-Status' not in res.headers)

    def test_omits_header_if_username_too_long(self):
        res = self.__authenticated_request(username_too_long)
        self.assertEqual(200, res.code)
        self.assertTrue('X-Account-Management-Status' not in res.headers)
