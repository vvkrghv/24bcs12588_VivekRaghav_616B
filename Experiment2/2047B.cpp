#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int t;
    cin >> t;

    while (t--) {
        int n;
        cin >> n;
        string s;
        cin >> s;

        vector<int> freq(26, 0);
        for (char c : s) {
            freq[c - 'a']++;
        }
        int maxIdx = 0;
        for (int i = 0; i < 26; i++) {
            if (freq[i] > freq[maxIdx]) {
                maxIdx = i;
            }
        }
        int minIdx = -1;
        for (int i = 0; i < 26; i++) {
            if (freq[i] > 0 && i != maxIdx &&
                (minIdx == -1 || freq[i] < freq[minIdx])) {
                minIdx = i;
            }
        }
        if (minIdx == -1) {
            cout << s << '\n';
            continue;
        }

        char maxChar = char('a' + maxIdx);
        char minChar = char('a' + minIdx);

        for (int i = 0; i < n; i++) {
            if (s[i] == minChar) {
                s[i] = maxChar;
                break;
            }
        }

        cout << s << '\n';
    }

    return 0;
}
