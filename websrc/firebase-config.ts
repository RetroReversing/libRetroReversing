// Import the functions you need from the SDKs you need
import { initializeApp } from "firebase/app";
import { getAnalytics } from "firebase/analytics";
// TODO: Add SDKs for Firebase products that you want to use
// https://firebase.google.com/docs/web/setup#available-libraries
import { getFirestore} from "@firebase/firestore";

// Your web app's Firebase configuration
// For Firebase JS SDK v7.20.0 and later, measurementId is optional
const firebaseConfig = {
  apiKey: "AIzaSyAolPRDKHKqwcBrbb6N-9u7uFxN38FQ2KE",
  authDomain: "reversingemulator.firebaseapp.com",
  projectId: "reversingemulator",
  storageBucket: "reversingemulator.appspot.com",
  messagingSenderId: "596902357954",
  appId: "1:596902357954:web:1a2a2941ce7e3ee008845b",
  measurementId: "G-D93PXBB0NH"
};

// Initialize Firebase
const app = initializeApp(firebaseConfig);
const analytics = getAnalytics(app);

export const db = getFirestore(app);