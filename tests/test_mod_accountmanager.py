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
        self.protected_url = "http://test-mod-accountmanager.mutualauth.org/protected/"
        self.public_url = "http://test-mod-accountmanager.mutualauth.org/hello.sh"
        self.amcd_url = "http://test-mod-accountmanager.mutualauth.org/amcd.json"
        self.realm = "mod_accountmanager test"

    def assertHasLinkHeader(self, res):
        self.assertEqual('<%s>; rel="acct-mgmt"' % self.amcd_url,
                         res.headers['Link'])
        
    def test_unauthenticated_request_of_protected_resource(self):
        res = None
        try:
            res = urllib2.urlopen(self.protected_url)
            self.assertTrue(False, "urlopen should've failed -- requires auth")
        except urllib2.HTTPError as err:
            self.assertEqual(401, err.code)
            self.assertTrue('X-Account-Management-Status' not in err.headers)
            
            # TODO - assert below is failing, want even 401s to have Link hdr
            self.assertHasLinkHeader(err)


    def test_unauthenticated_request_of_public_resource(self):
        res = urllib2.urlopen(self.public_url)
        self.assertEqual(200, res.code)
        self.assertEqual('none', res.headers['X-Account-Management-Status'])
        self.assertHasLinkHeader(res)

    def __authenticated_request(self, user):
        auth_handler = urllib2.HTTPBasicAuthHandler()
        auth_handler.add_password(realm=self.realm, uri=self.protected_url,
                                  user=user, passwd=passwd[user])
        opener = urllib2.build_opener(auth_handler)
        res = opener.open(self.protected_url)
        return res

    def test_authenticated_request(self):
        res = self.__authenticated_request('jsmith')
        self.assertEqual('active; id="jsmith"',
                         res.headers['X-Account-Management-Status'])
        self.assertHasLinkHeader(res)

    def test_omits_header_if_username_has_invalid_chars(self):
        res = self.__authenticated_request(username_invalid_chars)
        self.assertEqual(200, res.code)
        self.assertTrue('X-Account-Management-Status' not in res.headers)
        self.assertHasLinkHeader(res)

    def test_omits_header_if_username_too_long(self):
        res = self.__authenticated_request(username_too_long)
        self.assertEqual(200, res.code)
        self.assertTrue('X-Account-Management-Status' not in res.headers)
        self.assertHasLinkHeader(res)

    def test_doesnt_clobber_other_handlers(self):
        # hello.sh is handled by `AddHandler cgi-script .sh`
        res = urllib2.urlopen(self.public_url)
        self.assertEqual("hello\n", res.read())
